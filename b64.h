#ifndef __BASE64_H
#define __BASE64_H

#include <stdio.h>

#define BASE64_LENGTH(inlen) ((((inlen) + 2) / 3) * 4)

#define B64_DEF_LINE_SIZE   72
#define BAD     -1
#define DECODE64(c)  (isascii(c) ? base64val[c] : BAD)

static const char base64digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

typedef struct {
	int	remlen;
	int	rem[3];
} base64_state_t; 

void base64_init(base64_state_t *stat);
char *base64_append(base64_state_t *stat, char *in , size_t in_len, size_t *out_len, int mode);

#endif

