/*
 * main.c
 * This file is part of str2hex
 *
 * Copyright (C) 2008 - Dmitry Plashchynski <dmitry@plashchynski.net>
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



static void print_version(void);	// print version and copyright information and exit.
static void usage(void);		// print usage information
static void set_mode(int majour_mode, int minour_mode, struct _config *config);
static void split_fwrite(char *out_buffer, int sz, int out_buffer_size, FILE *out_file,  int linesz);
static void config_init(struct _config *config);

int main(int argc, char **argv)
{
	int	i, i2;		// cycle indexers

	struct _config config;

	config_init(&config);

	FILE 		* in_file, * out_file;
	in_file = stdin;
	out_file = stdout;
	
	unsigned char	*in = NULL;	// input buffer;

	if (argc == 1)	// too few args - print usage and exit
	{
		usage();
		exit(0);
	}

	int option_index = 0;
	int c = 0;

	static struct option long_options[] = {
		{"string", 2, 0, 's'},
		{"if", 1, 0, 'f'},
		{"of", 2, 0, 'o'},
		{"nlign", 0, 0, 'q'},
		{"include", 1, 0, 'i'},
		{"exclude", 1, 0, 'e'},
		{"help", 0, 0, '?'},
		{"version", 0, 0, 'v'},
		{"p",0,0,'p'},
		{"no",0,0,2},
		{"t",0,0,'t'},
		{"tc",0,0,3},
		{"tp",0,0,4},
		{"a",0,0,'a'},
		{"ac",0,0,5},
		{"ap",0,0,6},
		{"m",0,0,'m'},
		{"mc",0,0,7},
		{"x",0,0,'h'},
		{"xe",0,0,8},
		{"xw",0,0,9},
		{"w",0,0,'w'},
		{"b64",2,0,'b'},
		{"base64",2,0,'b'},
		{"bn",0,0,14},
		{"cf",0,0,10},
		{"cc",0,0,11},
		{"ch",0,0,12},
		{"md5",0,0,13},
		{0, 0, 0, 0}
	};

	opterr = 0;

	while((c = getopt_long_only(argc, argv, "-s::f:bo:qi:e:hvptanmxw",
		long_options, &option_index)) != -1)

	switch(c)
	{
		// read input from string
		case 's':
			if (config.from == 1)
				exit_error("Too many \'-s\' arguments!");

			if (config.from == 2)
				exit_error("Too many input points! You defined \'-s\' together with \'-f\'!");

			config.from = 1;
			
			if (optarg)
			{
				in = malloc(strlen(optarg) * sizeof(char));
				strcpy((char*)in, optarg);
			}
			
			break;


		// read input from file
		case 'f':
			if (config.from == 2)
				exit_error("Too many \'-f\' arguments!");
			
			if (config.from == 1)
				exit_error("Too many input points! You define \'-f\' together with \'-s\'!");

			config.from = 2;
			
			// open input file
			if (optarg)
				if ((in_file = fopen(optarg,"rb")) == NULL)
					exit_error("Can\'t open input file!");
			break;


		// write output to file
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


		case 't':	// for output in AT&T assembler hex-char format: 0xF5, 0xF3, 0xE9...
			set_mode(1,0,&config);
			break;


		case 3: // for output in AT&T assembler without commas: 0xF5 0xF3
			set_mode(1,1,&config);
			break;


		case 4: // for putput in AT&T assembler hex-char plain format: 0xF50xF30xE9
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


		case 'x': // for output in HTML hex format: &#x6C;&#x6F;&#x78;&#x78;...
			set_mode(5,0,&config);
			break;


		case 8: // for output in HTML escape codes: &iexcl;&#x78;&copy;...
			set_mode(5,1,&config);
			break;


		case 9:  //  HTML escape codes, without semicolons: &#108&#111...
			set_mode(5,2,&config);
			break;


		case 'w':
			set_mode(6,0,&config);
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
			fprintf(stderr, "I don't know option \'%s\'\n", argv[optopt]);
			usage();
			exit(EXIT_FAILURE);
			break;


		case 1:
			if (in && config.from == 1)
				exit_error("Too many input string!");

			if (config.from == 2)
				exit_error("The argument \'-f\' has defined, you can't input string from command line arguments.");

			if (!config.from)
				config.from = 1;

			in = malloc(strlen(optarg) * sizeof(char));
			strcpy((char*)in, optarg);
			break;
	}

	if (!config.from)
		exit_error("Input string/points is not defined!");

	if (!config.mode)
		config.mode = 3;



///////////////////////////
//          processing           //
///////////////////////////
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
	printf( "str2hex - String to HEX many formats convertion utility.\n" \
		"str2hex version: %s\n" \
		"str2hex built:  %s %s\n" \
		"(C) Dmitry Plashchynski, 2007\n",VERSION,__TIME__, __DATE__);
	
	exit(EXIT_SUCCESS);
}


static void usage(void)
{
	printf( "Usage: str2hex [params] <ASCII Text>\n" \
		"   or: str2hex [params] \'<ASCII Text>\'\n\n" \
		"Params:\n" \
		"   -s <string>  : read input string from command line arguments.\n" \
		"   -f <file> : read input string from file - default: STDIN\n" \
		"   -o <file> : output in to file - default: STDOU\n" \
		"   -q   : ignore \"new line\" symbols in input, and use \"new line\" in output.\n" \
		"   -h  : print this help.\n" \
		"   -v   : print version and copyright information.\n" \
		"   -i [char],[char],[char]... : include to convert list only symbols \"char\" : ..%%2f..%%2fetc...\n" \
		"   -e [char],[char],[char]... : exclude from convert list symbols \"char\" : %%2e%%2e/%%2e%%2e...\n\n" \
		"Output format params:\n" \
		"   -p   : output in \"plain\" hex format: 2f6574632f706173737764...\n" \
		"   -n   : convert 10-base nums to 16-base (hex) nums: 6323A37B327F...\n" \
		"   -no  : convert 10-base nums to 8-base (oct) nums: 384636424...\n" \
		"   -m   : output in MySQL format: 0x2f6574632f706173737764... (default!)\n" \
		"   -mc  : output in MySQL CHAR format: CHAR(2f,65,74,63,2f,70)...\n" \
		"   -c   : output in C-style oct-chars format: \\3\\94\\94\\574\\545...\n" \
		"   -ch  : output in C-style hex-chars format: \\x2\\x94\\xE3\\x32...\n" \
		"   -cf  :  *  full printf suntax style: \\\"\\32\\322\\\"\\n...\n" \
		"   -cc  :  *  plain text printf suntax style: \\\"MS - suxx\\\"\\n\n" \
		"   -t   : output in AT&T assembler hex-charsformat: 0xF5, 0xF3, 0xE9...\n" \
		"   -tc  :  *  without commas: 0xF5 0xF3 0xE9\n" \
		"   -tp  :  *  \"plain\" format: 0xF50xF30xE9...\n" \
		"   -a   : output in Microsoft-Assembler chars format: E3h, C3h, E3h...\n" \
		"   -ac  :  *  without commas: E3h C3h E3h...\n" \
		"   -ap  :  * \"plain\" format: E3hC3hE3h...\n" \
		"   -u   : output in URL format: %%EF%%F0%%E5%%E2%%E5%%E4%%21...\n" \
		"   -x   : output in HTML hex format: &#x6C;&#x6F;&#x78;&#x78;...\n" \
		"   -xe  : output in HTML escape codes: &iexcl;&#178;&copy;...\n" \
		"   -xw  :  *  HTML escape codes, without semicolons: &#108&#111...\n" \		"   -b (-base64[=linesize] | -b64[=linesize] ) : output in Base64: YmZnYmRiZ2Q=\n" \
		"   -bn  : output in Base64, but do not newline formating.\n"
		"   -md5 : output MD5 (RFC 1321) hash: 929ae467fe43191eff23b9a0e1471d04\n\n" \
		"Exemples:\n" \
		"   str2hex bla-bla-bla\n" \
		"   str2hex -u \'-bla-bla bla\'\n" \
		"   str2hex -b64 -f /etc/passwd\n" \
		"   str2hex \'\'bla-bla bla\'\'\n" \
		"   str2hex -i 1,2,3,4,5,6,7,8,9,0 12345678910\n" \
		"   str2hex -a -e 1234567890abcde bsedtskdwnshc\n\n" );
}


void set_mode(int majour_mode, int minour_mode, struct _config *config)
{
	if (config->mode != 0)
		exit_error("Too many convertion mode arguments!");

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

