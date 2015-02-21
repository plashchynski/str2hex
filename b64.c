/*
 * b64.c
 * This file is part of str2hex project.
 *
 * Copyright 2005 Dzmitry Plashchynski <plashchynski@gmail.com>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include "b64.h"

void base64_init(base64_state_t *stat)
{
	stat->remlen = 0;
}

char *base64_append(base64_state_t *stat, char *in , size_t in_len, size_t *out_len, int mode)
{
	int i=0, i2=0;
	char *out = NULL;
	size_t outlen = 1 + BASE64_LENGTH(in_len);
	
	out = malloc(outlen);
	
	if (stat->remlen > 0)
	{
		out[i++] = base64digits[stat->rem[1] >> 2];
		out[i++] = base64digits[((stat->rem[1] << 4) & 0x30) | ((stat->remlen > 1 ? stat->rem[2] : in[i2]) >> 4)];
		out[i++] = base64digits[(((stat->remlen > 1 ? stat->rem[2] : in[i2++]) << 2) & 0x3c) |  \
			((stat->remlen > 2 ? stat->rem[3] : in[i2]) >> 6)];
		out[i++] = base64digits[(stat->remlen > 2 ? stat->rem[3] : in[i2++]) & 0x3f];	
	}

	for (; in_len >= 3; in_len -= 3)
	{
		out[i++] = base64digits[in[i2] >> 2];
		out[i++] = base64digits[((in[i2] << 4) & 0x30) | (in[i2+1] >> 4)];
		out[i++] = base64digits[((in[i2+1] << 2) & 0x3c) | (in[i2+2] >> 6)];
		out[i++] = base64digits[in[i2+2] & 0x3f];
		i2 += 3;

		if (outlen < i)
			out = realloc(out, outlen+=32);
	}
	
	if (in_len > 0)
	{
		if (!mode)
		{
			stat->remlen = in_len;
			for (;in_len > 0; in_len--)
				stat->rem[in_len] = in[in_len];
		} else
		{
			out = realloc(out, outlen+=32);

			unsigned char fragment;
			out[i++] = base64digits[in[i2] >> 2];
			fragment = (in[i2] << 4) & 0x30;
			if (in_len > 1)
				fragment |= in[i2+1] >> 4;
 
			out[i++] = base64digits[fragment];
			out[i++] = (in_len < 2) ? '=' : base64digits[(in[i2+1] << 2) & 0x3c];
			out[i++] = '=';
		}
	}

	*out_len = i;

	return out;
}
