/*
 * process.c
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
#include <string.h>
#include "main.h"
#include "process.h"
#include "md5.h"
#include "b64.h"


char *process(unsigned char *buf, size_t *out_size, size_t len, struct _config *config, int mode)
{
	static	int	gcount	= 0;
	register  int	i = 0;
	char	*out_buffer = NULL;
	static int ide=0;

	gcount++;

	/* Base64 */
	if (config->mode == 7)
	{
		static base64_state_t	b64_state;
		
		if (gcount == 1)
			base64_init(&b64_state);
		
		if (mode)
			out_buffer = base64_append(&b64_state, (char*) buf, len, out_size, 1);
		else
			out_buffer = base64_append(&b64_state, (char*) buf, len, out_size, 0);

		return out_buffer;	
	}

	/* Base10 number to Base16 numbers */
	if (config->mode == 10) /* -n and -no options */
	{
		int num = atoi((char*)buf);
		switch (config->mode2)
		{
			case 1: /* -n and -no options */
				out_buffer = malloc(strlen((char*)buf) * 2); /* -no options */
				*out_size += sprintf(out_buffer+*out_size,"%o", num);
				break;
			default:
				out_buffer = malloc(strlen((char*)buf) * 2); /* -n options */
				*out_size += sprintf(out_buffer+*out_size,"%x", num);
				break;
		}
		return out_buffer;
	}

#ifdef md5_INCLUDED
	/* MD5 */
	if (config->mode == 11)
	{
		// MD5 state structure
		static md5_state_t	md5_state;
		static int md5_mode = 0;
		static md5_byte_t	digest[16];
		
		
		if (!md5_mode)	/* first time true */
		{
			md5_init(&md5_state);
			mode = 1;
		}

		md5_append(&md5_state, (unsigned char*) buf, len);

		if (mode)	/* true at the end of computation */
		{
			out_buffer = malloc(sizeof(digest)*2+sizeof(char));
		
			md5_finish(&md5_state, digest);

			int	wrote=0;

			/* write binary hash in hex format */
			for (i = 0; i < 16; ++i)
				wrote += sprintf(out_buffer+wrote,"%02x", digest[i]);

			/* size of the result */
			*out_size = wrote;
			
			return out_buffer;
		}

		return NULL;
	}
#else
	if (config->mode == 11)
	{
		exit_error("MD5 encoding has been disabled on compilation time.");
		return NULL;
	}
#endif

	int	alloc_size = len;
	int	calc_alloc = 0;
	out_buffer = malloc(alloc_size *= 2);
	calc_alloc = alloc_size / 5 * 4;	/* 4/5 of the allocated memspace */

	/* char convertion alhoritm */
	for (i=0; i < len; i++)
	{
		if (*out_size > calc_alloc)	/* if used 4/5 of the allocated memspace */
		{
			alloc_size += calc_alloc/4;	/* increase 1/5 of the allocated memspace */
			out_buffer = realloc(out_buffer, alloc_size);
			calc_alloc = alloc_size / 5 * 4;	/* 4/5 of the allocated memspace */
		}
	
		/* include and exclude chars */
		if (config->exclude_symbols_size)
			if (memchr(config->exclude_symbols, buf[i],
				config->exclude_symbols_size))
					continue;

		if (config->include_symbols_size)
			if (!memchr(config->include_symbols, buf[i],
				config->include_symbols_size))
					{
						*out_size += sprintf(out_buffer+*out_size, "%c", buf[i]);
						continue;
					}

		/* Sort by CR. */
		if (config->nlign)
		{
#ifdef WIN32
			if ((buf[i] == '\n' && buf[i+1] == '\r') || (buf[i] == '\r' && buf[i+1] == '\n')	)
#else
			if (buf[i] == '\n')
#endif
			{
#ifdef WIN32
				*out_size+=sprintf(out_buffer+*out_size,"\r\n");
#else
				*out_size+=sprintf(out_buffer+*out_size,"\n");
#endif
				continue;
			}
		}

		/* Primary convertion method analys */
		switch(config->mode)
		{
			/* Plain hex style convertion */
			case 9:
				*out_size += sprintf(out_buffer+*out_size,"%02x",buf[i]);
				break;

			/* AT&T asm style convertion */
			case 1:
				if (i)
					switch (config->mode2)
					{
						case 1:
							*out_size += sprintf(out_buffer+*out_size," 0x%02x", buf[i]);
							break;
						case 2:
							*out_size += sprintf(out_buffer+*out_size,"0x%02x", buf[i]);
							break;
						default:
							*out_size += sprintf(out_buffer+*out_size,", 0x%02x", buf[i]);
					}
				else
					*out_size += sprintf(out_buffer+*out_size,"0x%02x", buf[i]);
				break;

			/* Microsoft assembler hex style */
			case 2:
				if (i)
					switch (config->mode2)
					{
						case 1:
							*out_size += sprintf(out_buffer+*out_size," %02xh", buf[i]);
							break;
						case 2:
							*out_size += sprintf(out_buffer+*out_size,"%02xh", buf[i]);
							break;
						default:
							*out_size += sprintf(out_buffer+*out_size,", %02xh", buf[i]);
					}
				else
					*out_size += sprintf(out_buffer+*out_size,"%02xh", buf[i]);
				break;

			/* MySQL Style convertion */
			case 3:
				switch(config->mode2)
				{
					case 1:
						if (ide)
						{
							if (i == len-1)
								*out_size += sprintf(out_buffer+*out_size,",%02x)", buf[i]);
							else
								*out_size += sprintf(out_buffer+*out_size,",%02x", buf[i]);
						} else
							*out_size += sprintf(out_buffer+*out_size,"CHAR(%02x", buf[i]);
						ide++;
					break;
					default:
						if (ide)
							*out_size += sprintf(out_buffer+*out_size,"%02x", buf[i]);
						else
							*out_size += sprintf(out_buffer+*out_size,"0x%02x", buf[i]);
						ide++;
				}
				break;


			/* URL format */
			case 4:
				*out_size += sprintf(out_buffer+*out_size,"%%%02x",buf[i]);
				break;

			/* HTML Style char convertion */
			case 5:
				switch (config->mode2)
				{
					case 1: // as HTML escape codes: &iexcl;&#x78;&copy;...
						switch (buf[i])
						{
							case 0x20:
								*out_size += sprintf(out_buffer+*out_size, "&nbsp;");
								break;
							case 0x22:
								*out_size += sprintf(out_buffer+*out_size, "&quot;");
								break;
							case 0x26:
								*out_size += sprintf(out_buffer+*out_size, "&amp;");
								break;
							case 0x2F:
								*out_size += sprintf(out_buffer+*out_size, "&frasl;");
								break;
							case 0x3C:
								*out_size += sprintf(out_buffer+*out_size, "&lt;");
								break;
							case 0x3E:
								*out_size += sprintf(out_buffer+*out_size, "&qt;");
								break;
							case 0x89:
								*out_size += sprintf(out_buffer+*out_size, "&permil;");
								break;
							case 0x8B:
								*out_size += sprintf(out_buffer+*out_size, "&lsaquo;");
								break;
							case 0x96:
								*out_size += sprintf(out_buffer+*out_size, "&ndash;");
								break;
							case 0x97:
								*out_size += sprintf(out_buffer+*out_size, "&mdash;");
								break;
							case 0x99:
								*out_size += sprintf(out_buffer+*out_size, "&trade;");
								break;
							case 0x9B:
								*out_size += sprintf(out_buffer+*out_size, "&rsaquo;");
								break;
							case 0xA1:
								*out_size += sprintf(out_buffer+*out_size, "&iexcl;");
								break;
							case 0xA2:
								*out_size += sprintf(out_buffer+*out_size, "&cent;");
								break;
							case 0xA3:
								*out_size += sprintf(out_buffer+*out_size, "&pound;");
								break;
							case 0xA4:
								*out_size += sprintf(out_buffer+*out_size, "&curren;");
								break;
							case 0xA5:
								*out_size += sprintf(out_buffer+*out_size, "&yen;");
								break;
							case 0xA6:
								*out_size += sprintf(out_buffer+*out_size, "&brvbar;");
								break;
							case 0xA7:
								*out_size += sprintf(out_buffer+*out_size, "&sect;");
								break;
							case 0xA8:
								*out_size += sprintf(out_buffer+*out_size, "&uml;");
								break;
							case 0xA9:
								*out_size += sprintf(out_buffer+*out_size, "&yen;");
								break;
							case 0xAA:
								*out_size += sprintf(out_buffer+*out_size, "&ordf;");
								break;
							case 0xAB:
								*out_size += sprintf(out_buffer+*out_size, "&laquo;");
								break;
							case 0xAC:
								*out_size += sprintf(out_buffer+*out_size, "&not;");
								break;
							case 0xAD:
								*out_size += sprintf(out_buffer+*out_size, "&shy;");
								break;
							case 0xAE:
								*out_size += sprintf(out_buffer+*out_size, "&reg;");
								break;
							case 0xAF:
								*out_size += sprintf(out_buffer+*out_size, "&macr;");
								break;
							case 0xB0:
								*out_size += sprintf(out_buffer+*out_size, "&deg;");
								break;
							case 0xB1:
								*out_size += sprintf(out_buffer+*out_size, "&plusmn;");
								break;
							case 0xB2:
								*out_size += sprintf(out_buffer+*out_size, "&sup2;");
								break;
							case 0xB3:
								*out_size += sprintf(out_buffer+*out_size, "&sup3;");
								break;
							case 0xB4:
								*out_size += sprintf(out_buffer+*out_size, "&acute;");
								break;
							case 0xB5:
								*out_size += sprintf(out_buffer+*out_size, "&micro;");
								break;
							case 0xB6:
								*out_size += sprintf(out_buffer+*out_size, "&para;");
								break;
							case 0xB7:
								*out_size += sprintf(out_buffer+*out_size, "&middot;");
								break;
							case 0xB8:
								*out_size += sprintf(out_buffer+*out_size, "&cedil;");
								break;
							case 0xB9:
								*out_size += sprintf(out_buffer+*out_size, "&sup1;");
								break;
							case 0xBA:
								*out_size += sprintf(out_buffer+*out_size, "&ordm;");
								break;
							case 0xBB:
								*out_size += sprintf(out_buffer+*out_size, "&raquo;");
								break;
							case 0xBC:
								*out_size += sprintf(out_buffer+*out_size, "&frac14;");
								break;
							case 0xBD:
								*out_size += sprintf(out_buffer+*out_size, "&frac12;");
								break;
							case 0xBE:
								*out_size += sprintf(out_buffer+*out_size, "&frac34;");
								break;
							case 0xBF:
								*out_size += sprintf(out_buffer+*out_size, "&iquest;");
								break;
							case 0xC0:
								*out_size += sprintf(out_buffer+*out_size, "&agrave;");
								break;
							case 0xC1:
								*out_size += sprintf(out_buffer+*out_size, "&Aacute;");
								break;
							case 0xC2:
								*out_size += sprintf(out_buffer+*out_size, "&Acirc;");
								break;
							case 0xC3:
								*out_size += sprintf(out_buffer+*out_size, "&Atilde;");
								break;
							case 0xC4:
								*out_size += sprintf(out_buffer+*out_size, "&Auml;");
								break;
							case 0xC5:
								*out_size += sprintf(out_buffer+*out_size, "&Aring;");
								break;
							case 0xC6:
								*out_size += sprintf(out_buffer+*out_size, "&AElig;");
								break;
							case 0xC7:
								*out_size += sprintf(out_buffer+*out_size, "&Ccedil;");
								break;
							case 0xC8:
								*out_size += sprintf(out_buffer+*out_size, "&Egrave;");
								break;
							case 0xC9:
								*out_size += sprintf(out_buffer+*out_size, "&Eacute;");
								break;
							case 0xCA:
								*out_size += sprintf(out_buffer+*out_size, "&Ecirc;");
								break;
							case 0xCB:
								*out_size += sprintf(out_buffer+*out_size, "&Euml;");
								break;
							case 0xCC:
								*out_size += sprintf(out_buffer+*out_size, "&Igrave;");
								break;
							case 0xCD:
								*out_size += sprintf(out_buffer+*out_size, "&Iacute;");
								break;
							case 0xCE:
								*out_size += sprintf(out_buffer+*out_size, "&Icirc;");
								break;
							case 0xCF:
								*out_size += sprintf(out_buffer+*out_size, "&Iuml;");
								break;
							case 0xD0:
								*out_size += sprintf(out_buffer+*out_size, "&ETH;");
								break;
							case 0xD1:
								*out_size += sprintf(out_buffer+*out_size, "&Ntilde;");
								break;
							case 0xD2:
								*out_size += sprintf(out_buffer+*out_size, "&Ograve;");
								break;
							case 0xD3:
								*out_size += sprintf(out_buffer+*out_size, "&Oacute;");
								break;
							case 0xD4:
								*out_size += sprintf(out_buffer+*out_size, "&Ocirc;");
								break;
							case 0xD5:
								*out_size += sprintf(out_buffer+*out_size, "&Otilde;");
								break;
							case 0xD6:
								*out_size += sprintf(out_buffer+*out_size, "&Ouml;");
								break;
							case 0xD7:
								*out_size += sprintf(out_buffer+*out_size, "&times;");
								break;
							case 0xD8:
								*out_size += sprintf(out_buffer+*out_size, "&Oslash;");
								break;
							case 0xD9:
								*out_size += sprintf(out_buffer+*out_size, "&Ugrave;");
								break;
							case 0xDA:
								*out_size += sprintf(out_buffer+*out_size, "&Uacute;");
								break;
							case 0xDB:
								*out_size += sprintf(out_buffer+*out_size, "&Ucirc;");
								break;
							case 0xDC:
								*out_size += sprintf(out_buffer+*out_size, "&Uuml;");
								break;
							case 0xDD:
								*out_size += sprintf(out_buffer+*out_size, "&Yacute;");
								break;
							case 0xDE:
								*out_size += sprintf(out_buffer+*out_size, "&THORN;");
								break;
							case 0xDF:
								*out_size += sprintf(out_buffer+*out_size, "&szlig;");
								break;
							case 0xE0:
								*out_size += sprintf(out_buffer+*out_size, "&agrave;");
								break;
							case 0xE1:
								*out_size += sprintf(out_buffer+*out_size, "&aacute;");
								break;
							case 0xE2:
								*out_size += sprintf(out_buffer+*out_size, "&acirc;");
								break;
							case 0xE3:
								*out_size += sprintf(out_buffer+*out_size, "&atilde;");
								break;
							case 0xE4:
								*out_size += sprintf(out_buffer+*out_size, "&auml;");
								break;
							case 0xE5:
								*out_size += sprintf(out_buffer+*out_size, "&aring;");
								break;
							case 0xE6:
								*out_size += sprintf(out_buffer+*out_size, "&aelig;");
								break;
							case 0xE7:
								*out_size += sprintf(out_buffer+*out_size, "&ccedil;");
								break;
							case 0xE8:
								*out_size += sprintf(out_buffer+*out_size, "&egrave;");
								break;
							case 0xE9:
								*out_size += sprintf(out_buffer+*out_size, "&eacute;");
								break;
							case 0xEA:
								*out_size += sprintf(out_buffer+*out_size, "&ecirc;");
								break;
							case 0xEB:
								*out_size += sprintf(out_buffer+*out_size, "&euml;");
								break;
							case 0xEC:
								*out_size += sprintf(out_buffer+*out_size, "&igrave;");
								break;
							case 0xED:
								*out_size += sprintf(out_buffer+*out_size, "&iacute;");
								break;
							case 0xEE:
								*out_size += sprintf(out_buffer+*out_size, "&icirc;");
								break;
							case 0xEF:
								*out_size += sprintf(out_buffer+*out_size, "&iuml;");
								break;
							case 0xF0:
								*out_size += sprintf(out_buffer+*out_size, "&eth;");
								break;
							case 0xF1:
								*out_size += sprintf(out_buffer+*out_size, "&ntilde;");
								break;
							case 0xF2:
								*out_size += sprintf(out_buffer+*out_size, "&ograve;");
								break;
							case 0xF3:
								*out_size += sprintf(out_buffer+*out_size, "&oacute;");
								break;
							case 0xF4:
								*out_size += sprintf(out_buffer+*out_size, "&ocirc;");
								break;
							case 0xF5:
								*out_size += sprintf(out_buffer+*out_size, "&otilde;");
								break;
							case 0xF6:
								*out_size += sprintf(out_buffer+*out_size, "&ouml;");
								break;
							case 0xF7:
								*out_size += sprintf(out_buffer+*out_size, "&divide;");
								break;
							case 0xF8:
								*out_size += sprintf(out_buffer+*out_size, "&oslash;");
								break;
							case 0xF9:
								*out_size += sprintf(out_buffer+*out_size, "&ugrave;");
								break;
							case 0xFA:
								*out_size += sprintf(out_buffer+*out_size, "&uacute;");
								break;
							case 0xFB:
								*out_size += sprintf(out_buffer+*out_size, "&ucirc;");
								break;
							case 0xFC:
								*out_size += sprintf(out_buffer+*out_size, "&uuml;");
								break;
							case 0xFD:
								*out_size += sprintf(out_buffer+*out_size, "&yacute;");
								break;
							case 0xFE:
								*out_size += sprintf(out_buffer+*out_size, "&thorn;");
								break;
							case 0xFF:
								*out_size += sprintf(out_buffer+*out_size, "&yuml;");
								break;
							default:
								*out_size += sprintf(out_buffer+*out_size,"&#%d;",buf[i]);
						}
						break;
					case 2:
						*out_size += sprintf(out_buffer+*out_size,"&#%d",buf[i]); 
						break;
					default:
						*out_size += sprintf(out_buffer+*out_size,"&#x%x",buf[i]); /* HTML hex-format */
				}
				break;


			/* C-style char convertion */
			case 8: 
				switch (config->mode2)
				{
					case 1:
					case 2:
						switch (buf[i])
						{
							case '\n':
								*out_size += sprintf(out_buffer+*out_size, "\\n");
								break;
							case '"':
								*out_size += sprintf(out_buffer+*out_size, "\\\"");
								break;
							case '\'':
								*out_size += sprintf(out_buffer+*out_size, "\\\'");
								break;
							case '%':
								*out_size += sprintf(out_buffer+*out_size, "%%");
								break;
							case '\\' :
								*out_size += sprintf(out_buffer+*out_size, "\\\\");
								break;
							case '\t':
								*out_size += sprintf(out_buffer+*out_size, "\\t");
								break;
							case '\v':
								*out_size += sprintf(out_buffer+*out_size, "\\v");
								break;
							case '\b':
								*out_size += sprintf(out_buffer+*out_size, "\\b");
								break;
							case '\r':
								*out_size += sprintf(out_buffer+*out_size, "\\r");
								break;
							case '\f':
								*out_size += sprintf(out_buffer+*out_size, "\\f");
								break;
							case '\a':
								*out_size += sprintf(out_buffer+*out_size, "\\a");
								break;
							default:
								if (config->mode2 == 1)
									*out_size += sprintf(out_buffer+*out_size,"\\%o",buf[i]);
								else
									*out_size += sprintf(out_buffer+*out_size, "%c", buf[i]);
						}
						break;
					case 3:
						*out_size += sprintf(out_buffer+*out_size,"\\x%x",buf[i]);
						break;
					default:
						*out_size += sprintf(out_buffer+*out_size,"\\%o",buf[i]);
				}
				break;
			default:
				*out_size += sprintf(out_buffer+*out_size,"%02x", buf[i]);
		}
	}

	return out_buffer;
}
