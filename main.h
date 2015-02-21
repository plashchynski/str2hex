#ifndef __MAIN_H
#define __MAIN_H

struct _config {
	int	from; 					// 1 - read from command argument string.
		           				// 2 - read from file.
	int	out;	   				// 1 - write out to the file.
	int	nlign;					// 1 - to ignore new line symbols in input stream.
	char	*include_symbols;
	int	include_symbols_size;
	char	*exclude_symbols;
	int	exclude_symbols_size;
	int	mode;						// convertion major mode
	int	mode2;					// convertion minor mode
	int	linesize;				// line size for base64
};

void exit_error(char *message); // print error message and exit.

#endif
