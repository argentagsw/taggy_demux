#ifndef _PROCESS_H_
#define _PROCESS_H_

void *process_file(globals_t *g);
//void process_sam_entries(globals_t *g, read_t *read);
void process_fastq_entries(globals_t *g, read_t *read);
//void process_fasta_entries(globals_t *g, read_t *read);
ssize_t process_fastq_entry(globals_t *g);
void collect_stats(globals_t *dest, const globals_t *src);

#endif

