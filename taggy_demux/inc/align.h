#ifndef _ALIGN_H_
#define _ALIGN_H_

//#ifdef __cplusplus
//extern "C"
//{
//#endif

void align_wrapper(globals_t *g, hit_t *hits);
ssize_t load_primers(argopts_t *p);
void sort_primers(argopts_t *p);
void print_primers(const argopts_t *p);
void clean_primers(argopts_t *p);

//#ifdef __cplusplus
//}
//#endif

#endif

