/*
 * b64.c
 * This file is part of str2hex
 *
 * Copyright (C) 2008 - Dmitry Plashchynski <plashchynski@gmail.com>
 *
 * str2hex is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * str2hex is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with str2hex; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
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

