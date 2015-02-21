/*
 * main.c
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
#include <ctype.h>
#include <getopt.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include "main.h"
#include "version.h"
#include "process.h"
#include "b64.h"


static void print_version(void);	/* print version, copyright information and exit. */
static void usage(void);					/* print usage */
static void set_mode(int majour_mode, int minour_mode, struct _config *config);
static void split_fwrite(char *out_buffer, int sz, int out_buffer_size, FILE *out_file,  int linesz);
static void config_init(struct _config *config);

static void usage(void)
{
	printf( "Usage: str2hex [params] <string>\n" \
		"   or: str2hex [params] \'<string>\'\n\n" \
		"Params:\n" \
		"   -f <file>	Read input from the file (Default is STDIN).\n" \
		"   -o <file>	Output to the file (Default is STDOU).\n" \
		"   -q 		Ignore \"new line\" symbols.\n" \
		"   -h 		This help.\n" \
		"   -v   	Version.\n" \
		"   -i [char],[char],[char]...	Include to convert list only symbols \"char\" : ..%%2f..%%2fetc...\n" \
		"   -e [char],[char],[char]...	Exclude from convert list symbols \"char\" : %%2e%%2e/%%2e%%2e...\n\n" \
		"Conversion params:\n" \
		"   -p 		Convert to \"plain\" hex format: 2f6574632f706173737764...\n" \
		"   -n 		Convert 10-base numbers to 16-base (hex) numbers: 6323A37B327F...\n" \
		"   -no 		Convert 10-base numbers to 8-base (oct) numbers: 384636424...\n" \
		"   -m 		Convert to MySQL format: 0x2f6574632f706173737764... (Default)\n" \
		"   -mc 		Output as MySQL CHAR format: CHAR(2f,65,74,63,2f,70)...\n" \
		"   -c 		Convert to C-style oct-chars format: \\3\\94\\94\\574\\545...\n" \
		"   -ch  	Convert to C-style hex-chars format: \\x2\\x94\\xE3\\x32...\n" \
		"   -cf  	*  full printf suntax style: \\\"\\32\\322\\\"\\n...\n" \
		"   -cc  	*  plain text printf suntax style: \\\"Lorem ipsum\\\"\\n\n" \
		"   -t   	Convert to AT&T assembler hex-chars format: 0xF5, 0xF3, 0xE9...\n" \
		"   -tc  	*  without commas: 0xF5 0xF3 0xE9\n" \
		"   -tp  	*  \"plain\" format: 0xF50xF30xE9...\n" \
		"   -a   	Convert to Microsoft-Assembler chars format: E3h, C3h, E3h...\n" \
		"   -ac  	*  without commas: E3h C3h E3h...\n" \
		"   -ap  	* \"plain\" format: E3hC3hE3h...\n" \
		"   -u   	Convert to URL format: %%EF%%F0%%E5%%E2%%E5%%E4%%21...\n" \
		"   -x   	Convert to HTML hex format: &#x6C;&#x6F;&#x78;&#x78;...\n" \
		"   -xe  	Convert to HTML escape codes: &iexcl;&#178;&copy;...\n" \
		"   -xw  	*  HTML escape codes, without semicolons: &#108&#111...\n" \
		"   -b (-base64[=linesize] | -b64[=linesize] )		Output in Base64: YmZnYmRiZ2Q=\n" \
		"   -bn  	Convert to Base64, but without newline formating.\n" \
		"   -md5 	Calculate MD5 (RFC 1321) hash: 929ae467fe43191eff23b9a0e1471d04\n\n" \
		"Exemples:\n" \
		"   str2hex \'Lorem ipsum\'\n" \
		"   str2hex -u \'Lorem ipsum\'\n" \
		"   str2hex -b64 -f /etc/passwd\n" \
		"   str2hex -i 1,2,3,4,5,6,7,8,9,0 12345678910\n" \
		"   str2hex -a -e 1234567890abcde bsedtskdwnshc\n\n" );
}

int main(int argc, char **argv)
{
	int	i, i2;

	struct _config config;

	config_init(&config);

	FILE 		* in_file, * out_file;
	in_file = stdin;
	out_file = stdout;
	
	unsigned char	*in = NULL;	/* input buffer */

	if (argc == 1)	/* few args - print usage and exit */
	{
		usage();
		exit(0);
	}

	int option_index = 0;
	int c = 0;

	static struct option long_options[] = {
		{"f", 1, 0, 'f'},
		{"o", 2, 0, 'o'},
		{"q", 0, 0, 'q'},
		{"help", 0, 0, '?'},
		{"version", 0, 0, 'v'},
		{"i", 1, 0, 'i'},
		{"e", 1, 0, 'e'},
		{"p",0,0,'p'},
		{"n",0,0,'n'},
		{"no",0,0,2},
		{"m",0,0,'m'},
		{"mc",0,0,7},
		{"c",0,0,'c'},
		{"ch",0,0,12},
		{"cf",0,0,10},
		{"cc",0,0,11},
		{"t",0,0,'t'},
		{"tc",0,0,3},
		{"tp",0,0,4},
		{"a",0,0,'a'},
		{"ac",0,0,5},
		{"ap",0,0,6},
		{"u",0,0,'u'},
		{"x",0,0,'x'},
		{"xe",0,0,8},
		{"xw",0,0,9},
		{"b64",2,0,'b'},
		{"base64",2,0,'b'},
		{"bn",0,0,14},
		{"md5",0,0,13},
		{0, 0, 0, 0}
	};

	opterr = 0;

	while((c = getopt_long_only(argc, argv, "-f:o:qi:e:hvptabnmxwu",
		long_options, &option_index)) != -1)

		switch(c)
		{
			/* read input from file */
			case 'f':
				if (config.from == 2)
					exit_error("Too many \'-f\' arguments!");

				config.from = 2;

				/* open input file */
				if (optarg)
					if ((in_file = fopen(optarg,"rb")) == NULL)
						exit_error("Can\'t open the input file.");
				break;

			/* write output to file */
			case 'o':
				if (config.out == 1)
					exit_error("Too many \'-o\' arguments!");	
		
				config.out = 1;
		
				if ((out_file = fopen(optarg,"w")) == NULL)
					exit_error("Can\'t open output file!");
				break;

			case 'q':
				if (config.nlign == 1)
					exit_error("Too many \'-q\' arguments!");

				config.nlign = 1;
				break;

			case 'i':
				config.include_symbols = malloc(strlen(optarg) * sizeof(char));
				for (i = 0, i2 = 0; i< strlen(optarg); i++)
					if (optarg[i] != ',') // ignore separate symbol
						config.include_symbols[i2++] = optarg[i];
				config.include_symbols_size = i2;
				break;

			case 'e':
				config.exclude_symbols = malloc(strlen(optarg) * sizeof(char));
				for (i = 0, i2 = 0; i<strlen(optarg); i++)
					if (optarg[i] != ',')	// ignore separate symbol
						config.exclude_symbols[i2++] = optarg[i];
				config.exclude_symbols_size = i2;
				break;

			case 'h':
				usage();
				exit(EXIT_SUCCESS);
				break;

			case 'v':
				print_version();
				break;

			case 'p':
				set_mode(9,0,&config);
				break;

			case 'n':
				set_mode(10,0,&config);
				break;

			case 2:
				set_mode(10,1,&config);
				break;

			case 't':	/* for output in AT&T assembler hex-char format: 0xF5, 0xF3, 0xE9... */
				set_mode(1,0,&config);
				break;

			case 3: /* for output in AT&T assembler without commas: 0xF5 0xF3 */
				set_mode(1,1,&config);
				break;

			case 4: /* for putput in AT&T assembler hex-char plain format: 0xF50xF30xE9 */
				set_mode(1,2,&config);
				break;

			case 'a':
				set_mode(2,0,&config);
				break;

			case 5:
				set_mode(2,1,&config);
				break;

			case 6:
				set_mode(2,2,&config);
				break;

			case 'm':
				set_mode(3,0,&config);
				break;

			case 7:
				set_mode(3,1,&config);
				break;

			case 'u':
				set_mode(4,0,&config);
				break;

			case 'x': /* for output in HTML hex format: &#x6C;&#x6F;&#x78;&#x78;... */
				set_mode(5,0,&config);
				break;

			case 8: /* for output in HTML escape codes: &iexcl;&#x78;&copy;... */
				set_mode(5,1,&config);
				break;

			case 9:  /*  HTML escape codes, without semicolons: &#108&#111... */
				set_mode(5,2,&config);
				break;

			case 'b':
				if (optarg)
					config.linesize = atoi(optarg);
				else
					config.linesize = B64_DEF_LINE_SIZE;
		
				set_mode(7,0,&config);
				break;

			case 'c':
				set_mode(8,0,&config);
				break;

			case 10:
				set_mode(8,1,&config);
				break;

			case 11:
				set_mode(8,2,&config);
				break;

			case 12:
				set_mode(8,3,&config);
				break;

			case 13:
				set_mode(11,0,&config);
				break;
			
			case 14:
				set_mode(7,1,&config);
				break;

			case '?':
#if defined(BSD) || defined(__MACH__)
					fprintf(stderr, "Unknow option.\n");
#else
					fprintf(stderr, "Unknow option \'%s\'\n", argv[optopt]);
#endif
				usage();
				exit(EXIT_FAILURE);
				break;

			case 1:
				if (in && config.from == 1)
					exit_error("Please wrap the string in quotes: str2hex \"a few word string\".");

				if (config.from == 2)
					exit_error("You can't use both \'-f\' and string as the command line argument.");

				if (!config.from)
					config.from = 1;

				in = malloc(strlen(optarg) * sizeof(char));
				strcpy((char*)in, optarg);
				break;
		}

	if (!config.from)
		exit_error("You didn't provide any data to convert");

	if (!config.mode)
		config.mode = 3;

	/* Processing */
	if (config.from == 2)
	{
		#ifndef WIN32
			int page_size = sysconf(_SC_PAGESIZE);
			if (page_size == -1)
				page_size = 4096;
		#else
			int page_size = 4096;
		#endif
		
		size_t	in_buffer_size = page_size, out_buffer_size = 0, readsiz = 0;
		unsigned char	*in_buffer = malloc(in_buffer_size * sizeof(char));
		char	*out_buffer = NULL;
	
		while (!feof(in_file) && !ferror(in_file) && !ferror(out_file))
		{
			readsiz = fread(in_buffer, sizeof(char), in_buffer_size, in_file);
			
			if (!feof(in_file))
				out_buffer = process(in_buffer, &out_buffer_size, readsiz, &config, 0);
			else
				out_buffer = process(in_buffer, &out_buffer_size, readsiz, &config, 1);
			
			
			if (out_buffer_size)
			{
				if (config.mode == 7 && config.mode2 != 1)
					split_fwrite(out_buffer, sizeof(char), out_buffer_size, out_file, config.linesize);
				else
					fwrite(out_buffer, sizeof(char), out_buffer_size, out_file);
			}
			free(out_buffer);
		}

	} else
	{
		size_t len = strlen((char*)in);
		size_t out_buffer_size = 0;

		char *out_buffer = process(in, &out_buffer_size, len, &config, 1);
		
		if (out_buffer_size)
				fwrite(out_buffer, sizeof(char), out_buffer_size, out_file);	
	}

#ifdef WIN32
	fputs("\r\n",out_file);
#else
	fputc('\n',out_file);
#endif

	fclose(in_file);
	fclose(out_file);

	return(0);
}


static void print_version(void)
{
	printf( "str2hex - Data formats convertion utility.\n" \
		"str2hex version: %s\n" \
		"str2hex built:  %s %s\n" \
		"(C) 2005 Dzmitry Plashchynski <plashchynski@gmail.com>\n", VERSION, __TIME__, __DATE__);
	
	exit(EXIT_SUCCESS);
}

void set_mode(int majour_mode, int minour_mode, struct _config *config)
{
	if (config->mode != 0)
		exit_error("Too many convertion modes!");

	config->mode = majour_mode;
	config->mode2 = minour_mode;
}

void exit_error(char *message)
{
	fprintf(stderr,"ERROR: %s\n",message);
	exit(EXIT_FAILURE);
}

static void split_fwrite(char *out_buffer, int sz, int out_buffer_size, FILE *out_file, int linesz)
{
	int i;
	char	*buffer = out_buffer;
	static	int rem_size = 0;

	if (rem_size)
	{
#ifdef WIN32
		fputs("\r\n",out_file);
#else
	 	fputc('\n',out_file);
#endif

		buffer += rem_size;
	}

	for (i = out_buffer_size; i >= linesz; i -= linesz)
	{
		fwrite(buffer, sz, linesz, out_file);

#ifdef WIN32
		fputs("\r\n",out_file);
#else
	 	fputc('\n',out_file);
#endif

		buffer += linesz;
	}
	
	if (i > 0)
	{
		rem_size = linesz-i;

		fwrite(buffer, sz, i, out_file);
	}
}

static void config_init(struct _config *config)
{
	memset(config, 0, sizeof(struct _config));
	
	config->include_symbols_size = 0;
	config->exclude_symbols_size = 0;
	config->from = 0;
	config->mode = 0;
}

