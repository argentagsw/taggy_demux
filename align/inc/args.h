#ifndef ARGS_H
#define ARGS_H

#include <stdlib.h>
#include "ktypes.h"

void arg_parse(int argc, char *argv[], argopts_t *p);
void arg_post (argopts_t *p, files_t *files);
void arg_print();

#endif

