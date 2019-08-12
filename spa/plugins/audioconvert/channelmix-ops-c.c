/* Spa
 *
 * Copyright © 2018 Wim Taymans
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "channelmix-ops.h"

void
channelmix_copy_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t i, n;
	float **d = (float **)dst;
	const float **s = (const float **)src;
	const float *m = mix->matrix;
	const float v = mix->volume;

	if (v <= VOLUME_MIN) {
		for (i = 0; i < n_dst; i++)
			memset(d[i], 0, n_samples * sizeof(float));
	}
	else if (v == VOLUME_NORM) {
		for (i = 0; i < n_dst; i++)
			spa_memcpy(d[i], s[i], n_samples * sizeof(float));
	}
	else {
		for (i = 0; i < n_dst; i++) {
			const float vol = m[i * n_src + i];
			for (n = 0; n < n_samples; n++)
				d[i][n] = s[i][n] * vol;
		}

	}
}

#define _M(ch)		(1UL << SPA_AUDIO_CHANNEL_ ## ch)

void
channelmix_f32_n_m_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t i, j, n;
	float **d = (float **) dst;
	const float **s = (const float **) src;
	const float *m = mix->matrix;

	for (n = 0; n < n_samples; n++) {
		for (i = 0; i < n_dst; i++) {
			float sum = 0.0f;
			for (j = 0; j < n_src; j++)
				sum += s[j][n] * m[i * n_src + j];
			d[i][n] = sum;
		}
	}
}

#define MASK_MONO	_M(FC)|_M(MONO)|_M(UNKNOWN)
#define MASK_STEREO	_M(FL)|_M(FR)|_M(UNKNOWN)

void
channelmix_f32_1_2_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t n;
	float **d = (float **)dst;
	const float **s = (const float **)src;
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		memset(d[0], 0, n_samples * sizeof(float));
		memset(d[1], 0, n_samples * sizeof(float));
	}
	else if (v == VOLUME_NORM) {
		for (n = 0; n < n_samples; n++)
			d[0][n] = d[1][n] = s[0][n];
	}
	else {
		for (n = 0; n < n_samples; n++)
			d[0][n] = d[1][n] = s[0][n] * v;
	}
}

void
channelmix_f32_2_1_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		   uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t n;
	float **d = (float **)dst;
	const float **s = (const float **)src;
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		memset(d[0], 0, n_samples * sizeof(float));
	}
	else {
		const float f = v * 0.5f;
		for (n = 0; n < n_samples; n++)
			d[0][n] = (s[0][n] + s[1][n]) * f;
	}
}

void
channelmix_f32_4_1_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		   uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t n;
	float **d = (float **)dst;
	const float **s = (const float **)src;
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		memset(d[0], 0, n_samples * sizeof(float));
	}
	else {
		const float f = v * 0.25f;
		for (n = 0; n < n_samples; n++)
			d[0][n] = (s[0][n] + s[1][n] + s[2][n] + s[3][n]) * f;
	}
}

void
channelmix_f32_3p1_1_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		   uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t n;
	float **d = (float **)dst;
	const float **s = (const float **)src;
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		memset(d[0], 0, n_samples * sizeof(float));
	}
	else {
		const float f = v * 0.5f;
		for (n = 0; n < n_samples; n++)
			d[0][n] = (s[0][n] + s[1][n] + s[2][n]) * f;
	}
}


#define MASK_QUAD	_M(FL)|_M(FR)|_M(RL)|_M(RR)|_M(UNKNOWN)

void
channelmix_f32_2_4_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		   uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t i, n;
	float **d = (float **)dst;
	const float **s = (const float **)src;
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		for (i = 0; i < n_dst; i++)
			memset(d[i], 0, n_samples * sizeof(float));
	}
	else if (v == VOLUME_NORM) {
		for (n = 0; n < n_samples; n++) {
			d[0][n] = d[2][n] = s[0][n];
			d[1][n] = d[3][n] = s[1][n];
		}
	}
	else {
		for (n = 0; n < n_samples; n++) {
			d[0][n] = d[2][n] = s[0][n] * v;
			d[1][n] = d[3][n] = s[1][n] * v;
		}
	}
}

#define MASK_3_1	_M(FL)|_M(FR)|_M(FC)|_M(LFE)
void
channelmix_f32_2_3p1_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		   uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t i, n;
	float **d = (float **)dst;
	const float **s = (const float **)src;
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		for (i = 0; i < n_dst; i++)
			memset(d[i], 0, n_samples * sizeof(float));
	}
	else if (v == VOLUME_NORM) {
		for (n = 0; n < n_samples; n++) {
			d[0][n] = s[0][n];
			d[1][n] = s[1][n];
			d[2][n] = (s[0][n] + s[1][n]) * 0.5f;
			d[3][n] = 0.0f;
		}
	}
	else {
		const float f = 0.5f * v;
		for (n = 0; n < n_samples; n++) {
			d[0][n] = s[0][n] * v;
			d[1][n] = s[1][n] * v;
			d[2][n] = (s[0][n] + s[1][n]) * f;
			d[3][n] = 0.0f;
		}
	}
}

#define MASK_5_1	_M(FL)|_M(FR)|_M(FC)|_M(LFE)|_M(SL)|_M(SR)|_M(RL)|_M(RR)
void
channelmix_f32_2_5p1_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		   uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t i, n;
	float **d = (float **)dst;
	const float **s = (const float **)src;
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		for (i = 0; i < n_dst; i++)
			memset(d[i], 0, n_samples * sizeof(float));
	}
	else if (v == VOLUME_NORM) {
		for (n = 0; n < n_samples; n++) {
			d[0][n] = d[4][n] = s[0][n];
			d[1][n] = d[5][n] = s[1][n];
			d[2][n] = (s[0][n] + s[1][n]) * 0.5f;
			d[3][n] = 0.0f;
		}
	}
	else {
		const float f = 0.5f * v;
		for (n = 0; n < n_samples; n++) {
			d[0][n] = d[4][n] = s[0][n] * v;
			d[1][n] = d[5][n] = s[1][n] * v;
			d[2][n] = (s[0][n] + s[1][n]) * f;
			d[3][n] = 0.0f;
		}
	}
}

/* FL+FR+FC+LFE+SL+SR -> FL+FR */
void
channelmix_f32_5p1_2_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		   uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t n;
	float **d = (float **) dst;
	const float **s = (const float **) src;
	const float *m = mix->matrix;
	const float clev = m[2];
	const float llev = m[3];
	const float slev = m[4];
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		memset(d[0], 0, n_samples * sizeof(float));
		memset(d[1], 0, n_samples * sizeof(float));
	}
	else if (v == VOLUME_NORM) {
		for (n = 0; n < n_samples; n++) {
			const float ctr = clev * s[2][n] + llev * s[3][n];
			d[0][n] = s[0][n] + ctr + (slev * s[4][n]);
			d[1][n] = s[1][n] + ctr + (slev * s[5][n]);
		}
	}
	else {
		for (n = 0; n < n_samples; n++) {
			const float ctr = clev * s[2][n] + llev * s[3][n];
			d[0][n] = (s[0][n] + ctr + (slev * s[4][n])) * v;
			d[1][n] = (s[1][n] + ctr + (slev * s[5][n])) * v;
		}
	}
}

/* FL+FR+FC+LFE+SL+SR -> FL+FR+FC+LFE*/
void
channelmix_f32_5p1_3p1_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		   uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t i, n;
	float **d = (float **) dst;
	const float **s = (const float **) src;
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		for (i = 0; i < n_dst; i++)
			memset(d[i], 0, n_samples * sizeof(float));
	}
	else {
		const float f1 = 0.5f * v;
		for (n = 0; n < n_samples; n++) {
			d[0][n] = (s[0][n] + s[4][n]) * f1;
			d[1][n] = (s[1][n] + s[5][n]) * f1;
			d[2][n] = s[2][n] * v;
			d[3][n] = s[3][n] * v;
		}
	}
}

/* FL+FR+FC+LFE+SL+SR -> FL+FR+RL+RR*/
void
channelmix_f32_5p1_4_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		   uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t i, n;
	float **d = (float **) dst;
	const float **s = (const float **) src;
	const float *m = mix->matrix;
	const float clev = m[2];
	const float llev = m[3];
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		for (i = 0; i < n_dst; i++)
			memset(d[i], 0, n_samples * sizeof(float));
	}
	else if (v == VOLUME_NORM) {
		for (n = 0; n < n_samples; n++) {
			const float ctr = s[2][n] * clev + s[3][n] * llev;
			d[0][n] = s[0][n] + ctr;
			d[1][n] = s[1][n] + ctr;
			d[2][n] = s[4][n];
			d[3][n] = s[5][n];
		}
	}
	else {
		for (n = 0; n < n_samples; n++) {
			const float ctr = s[2][n] * clev + s[3][n] * llev;
			d[0][n] = (s[0][n] + ctr) * v;
			d[1][n] = (s[1][n] + ctr) * v;
			d[2][n] = s[4][n] * v;
			d[3][n] = s[5][n] * v;
		}
	}
}

#define MASK_7_1	_M(FL)|_M(FR)|_M(FC)|_M(LFE)|_M(SL)|_M(SR)|_M(RL)|_M(RR)

/* FL+FR+FC+LFE+SL+SR+RL+RR -> FL+FR */
void
channelmix_f32_7p1_2_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		   uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t n;
	float **d = (float **) dst;
	const float **s = (const float **) src;
	const float *m = mix->matrix;
	const float clev = m[2];
	const float llev = m[3];
	const float slev = m[4];
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		memset(d[0], 0, n_samples * sizeof(float));
		memset(d[1], 0, n_samples * sizeof(float));
	}
	else if (v == VOLUME_NORM) {
		for (n = 0; n < n_samples; n++) {
			const float ctr = clev * s[2][n] + llev * s[3][n];
			d[0][n] = s[0][n] + ctr + (slev * (s[4][n] + s[6][n]));
			d[1][n] = s[1][n] + ctr + (slev * (s[5][n] + s[7][n]));
		}
	}
	else {
		for (n = 0; n < n_samples; n++) {
			const float ctr = clev * s[2][n] + llev * s[3][n];
			d[0][n] = (s[0][n] + ctr + (slev * (s[4][n] + s[6][n]))) * v;
			d[1][n] = (s[1][n] + ctr + (slev * (s[5][n] + s[6][n]))) * v;
		}
	}
}

/* FL+FR+FC+LFE+SL+SR+RL+RR -> FL+FR+FC+LFE*/
void
channelmix_f32_7p1_3p1_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		   uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t i, n;
	float **d = (float **) dst;
	const float **s = (const float **) src;
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		for (i = 0; i < n_dst; i++)
			memset(d[i], 0, n_samples * sizeof(float));
	}
	else {
		const float f1 = 0.5 * v;
		for (n = 0; n < n_samples; n++) {
			d[0][n] = s[0][n] + (s[4][n] + s[6][n]) * f1;
			d[1][n] = s[1][n] + (s[5][n] + s[7][n]) * f1;
			d[2][n] = s[2][n] * v;
			d[3][n] = s[3][n] * v;
		}
	}
}

/* FL+FR+FC+LFE+SL+SR+RL+RR -> FL+FR+RL+RR*/
void
channelmix_f32_7p1_4_c(struct channelmix *mix, uint32_t n_dst, void * SPA_RESTRICT dst[n_dst],
		   uint32_t n_src, const void * SPA_RESTRICT src[n_src], uint32_t n_samples)
{
	uint32_t i, n;
	float **d = (float **) dst;
	const float **s = (const float **) src;
	const float *m = mix->matrix;
	const float clev = m[2];
	const float llev = m[3];
	const float slev = m[4];
	float v = mix->volume;

	if (v <= VOLUME_MIN) {
		for (i = 0; i < n_dst; i++)
			memset(d[i], 0, n_samples * sizeof(float));
	}
	else if (v == VOLUME_NORM) {
		for (n = 0; n < n_samples; n++) {
			const float ctr = s[2][n] * clev + s[3][n] * llev;
			const float sl = s[4][n] * slev;
			const float sr = s[5][n] * slev;
			d[0][n] = s[0][n] + ctr + sl;
			d[1][n] = s[1][n] + ctr + sr;
			d[2][n] = s[6][n] + sl;
			d[3][n] = s[7][n] + sr;
		}
	}
	else {
		for (n = 0; n < n_samples; n++) {
			const float ctr = s[2][n] * clev + s[3][n] * llev;
			const float sl = s[4][n] * slev;
			const float sr = s[5][n] * slev;
			d[0][n] = (s[0][n] + ctr + sl) * v;
			d[1][n] = (s[1][n] + ctr + sr) * v;
			d[2][n] = (s[6][n] + sl) * v;
			d[3][n] = (s[7][n] + sr) * v;
		}
	}
}
