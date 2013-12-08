#include "synced_t2.h"

#include <math.h>
#include <string.h>

#include "stream.h"
#include "../modes.h"
#include "../error.h"

/*
The basic premise of the synced_t2 operation is to convert a stream t2 photons
to t3 photons, where one of the t2 signal channels is treated as a dedicated
sync channel. This means that that other signal channels are emitted as timed
relative to the sync channel, just as though the sync channel were used in
hardware. This circumvents the hardware limitations of the t3 mode, which
has a worse time resolution and lower relative data rate than t2 mode, at the
expense of extra storage and software overhead, since all sync ``photons'' are 
stored.

There are two ways for this to work:
1. Every sync photon is recorded.
2. Every nth sync photon is recorded (a sync divider is used)

In the case that there is a sync divider, we have two options:
1. Ignore the divider and use the recorded syncs (effectively repeats the 
   lifetime over the implicit syncs:
   |           | ---> |           |
2. Interpolate between recorded syncs to recover the implicit events:
   |           | ---> |   |   |   |

The first case is exactly the same as having an explicit sync (divider of 1),
and requires post-processing to bring the repetitions back onto each other. The
second case performs this step during processing.

In any case, we need to keep track of the index of the last sync event, as well
as its absolute time. From this, we can form the correlation of a signal
photon with the appropriate sync (explicit or implicit), which gives us the 
pulse number and relative time for the signal photon.
*/

int synced_t2_dispatch(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) { 
	return(synced_t2(stream_in, stream_out, options));
}

int synced_t2(FILE *stream_in, FILE *stream_out, pc_options_t const *options) {
	int result = PC_SUCCESS;
	synced_t2_t *synced_t2 = NULL;
	photon_stream_t *photon_stream = NULL;

	synced_t2 = synced_t2_alloc(options->queue_size);
	photon_stream = photon_stream_alloc(MODE_T2);
	
	if ( synced_t2 == NULL || photon_stream == NULL ) {
		result = PC_ERROR_MEM;
		error("Could not allocate memory for synced t2 run.");
	}

	if ( result == PC_SUCCESS ) {
		photon_stream_init(photon_stream, stream_in);
		synced_t2_init(synced_t2, options->sync_channel, options->sync_divider);

		while ( photon_stream_next_photon(photon_stream) == PC_SUCCESS ) {
			synced_t2_push(synced_t2, (t2_t *)photon_stream->photon);

			while ( synced_t2_next(synced_t2) == PC_SUCCESS )  {
				t3_fprintf(stream_out, &(synced_t2->photon));
			}
		}

		synced_t2_flush(synced_t2);
		while ( synced_t2_next(synced_t2) == PC_SUCCESS ) { 
			t3_fprintf(stream_out, &(synced_t2->photon));
		}
	}

	synced_t2_free(&synced_t2);
	photon_stream_free(&photon_stream);	

	return(result);
}

synced_t2_t *synced_t2_alloc(size_t const queue_size) {
	synced_t2_t *synced_t2 = NULL;

	synced_t2 = (synced_t2_t *)malloc(sizeof(synced_t2_t));

	if ( synced_t2 != NULL ) {
		synced_t2->queue = queue_alloc(sizeof(t2_t), queue_size);

		if ( synced_t2->queue == NULL ) {
			synced_t2_free(&synced_t2);
		}
	}

	return(synced_t2);
}

void synced_t2_free(synced_t2_t **synced_t2) {
	if ( *synced_t2 != NULL ) {
		queue_free(&(*synced_t2)->queue);
		free(*synced_t2);
		*synced_t2 = NULL;
	}
}

void synced_t2_init(synced_t2_t *synced_t2,
		unsigned int const sync_channel, unsigned int const sync_divider) {
	synced_t2->flushing = false;

	synced_t2->sync_index = -1;

	synced_t2->first_sync_seen = false;

	synced_t2->sync_channel = sync_channel;
	synced_t2->sync_divider = sync_divider;

	queue_init(synced_t2->queue);
}

int synced_t2_push(synced_t2_t *synced_t2, t2_t const *t2) {
	if ( t2->channel == synced_t2->sync_channel ) {
		if ( synced_t2->first_sync_seen ) {
			debug("New sync pulse\n");
			memcpy(&(synced_t2->last_sync), &(synced_t2->next_sync), 	
					sizeof(t2_t));
			memcpy(&(synced_t2->next_sync), t2, sizeof(t2_t));

			synced_t2_flush(synced_t2);
		} else {
			debug("First sync.\n");
			memcpy(&(synced_t2->next_sync), t2, sizeof(t2_t));

			synced_t2->first_sync_seen = true;
			synced_t2->sync_index++;
		}
	} else {
		if ( synced_t2->first_sync_seen ) { 
			queue_push(synced_t2->queue, t2);
		}
	}

	return(PC_SUCCESS);
}

void synced_t2_flush(synced_t2_t *synced_t2) {
	synced_t2->flushing = true;
}

int synced_t2_next(synced_t2_t *synced_t2) {
	t2_t photon;
	double sync_time, sync_delay, photon_delay, pulse, time;

	if ( synced_t2->flushing ) {
		if ( queue_empty(synced_t2->queue) ) {
			return(PC_ERROR_NO_RECORD_AVAILABLE);
		} else {
			queue_pop(synced_t2->queue, &photon);

			synced_t2->photon.channel = photon.channel;

			if ( synced_t2->sync_divider > 1 ) {
				sync_delay = synced_t2->next_sync.time - 
						synced_t2->last_sync.time;
				sync_time = sync_delay / synced_t2->sync_divider;
				photon_delay = photon.time - synced_t2->last_sync.time;

				time = modf(photon_delay/sync_time, &pulse) * sync_time;

				synced_t2->photon.pulse = 
						synced_t2->sync_index * synced_t2->sync_divider + 
						(unsigned long long)pulse;
				synced_t2->photon.time = (unsigned long long)round(time);
			} else {
				synced_t2->photon.pulse = synced_t2->sync_index;
				synced_t2->photon.time = photon.time 
						- synced_t2->last_sync.time;
			}

			if ( queue_empty(synced_t2->queue) ) {
				synced_t2->flushing = false;
				memcpy(&(synced_t2->last_sync), &(synced_t2->next_sync), 
						sizeof(t2_t));
				synced_t2->sync_index++;
			}

			return(PC_SUCCESS);
		}
	} else {
		return(PC_ERROR_NO_RECORD_AVAILABLE);
	}
}

