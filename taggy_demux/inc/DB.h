#ifndef _DB_H

#define _DB_H

#include <stdio.h>
#include <inttypes.h>

#define EPRINTF fprintf
#define EPLACE  stderr
#define EXIT(x) exit (1)

void *Malloc(const int64_t size, const char *mesg);
void *Realloc(void *object, int64_t size, const char *mesg);
char *Strdup(const char *string, const char *mesg); 

#endif

