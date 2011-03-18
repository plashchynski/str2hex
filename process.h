#ifndef __PROCESS_H
#define __PROCESS_H

#include "main.h"

char *process(unsigned char *buf, size_t *out_size, size_t len, struct _config *config, int mode);

#endif
