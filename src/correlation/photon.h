/*
 * Copyright (c) 2011-2015, Thomas Bischof
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Massachusetts Institute of Technology nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CORRELATION_PHOTON_H_
#define CORRELATION_PHOTON_H_

#include <stdio.h>
#include "../options.h"
#include "correlation.h"
#include "correlator.h"
#include "../combinatorics/combinations.h"

int correlate_photon(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);
int correlations_echo(FILE *stream_in, FILE *stream_out,
		pc_options_t const *options);

void t2_correlate(correlation_t *correlation);
int t2_correlation_fscanf(FILE *stream_in, correlation_t *correlation);
int t2_correlation_fprintf(FILE *stream_out, correlation_t const *correlation);

int t2_under_max_distance(correlator_t const *correlator);
int t2_over_min_distance(correlator_t const *correlator);

void t3_correlate(correlation_t *correlation);
int t3_correlation_fscanf(FILE *stream_in, correlation_t *correlation);
int t3_correlation_fprintf(FILE *stream_out, correlation_t const *correlation);

int t3_under_max_distance(correlator_t const *correlator);
int t3_over_min_distance(correlator_t const *correlator);

#endif

