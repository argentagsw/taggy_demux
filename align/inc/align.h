#ifndef _ALIGN_H_
#define _ALIGN_H_

void align_wrapper(globals_t *g);
ssize_t read_primers(const argopts_t *p);
void sort_primers(const argopts_t *p);
void print_primers(const argopts_t *p);
void clean_primers(const argopts_t *p);

#endif

