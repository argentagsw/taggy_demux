#ifndef _KTPYES_H_
#define _KTPYES_H_

#include <inttypes.h>
#include "time.h"

#define INPUT_TYPE_NONE                    0
#define INPUT_TYPE_FASTQ                   1

#define MAX_PRIMER_SIZE                 1024

typedef struct{
  ssize_t         seq_index;
  char           *line;
  size_t          linelen;
  char           *defline;
  char           *defline_short;
  char           *seq;
  char           *qv;
  ssize_t         average_dq, average_iq, average_sq, average_qv;
  ssize_t         seq_size;
  clock_t         start_read_time;
}read_t;

typedef struct{
  ssize_t   reads_total;
}stats_t;

typedef struct{
  FILE     *ifp;
}files_t;

typedef struct{
  char     *args[2];
  int       VERBOSE;
  int       INPUT_TYPE, NUM_PRIMERS, NUM_THREADS, PRINT_START, START_FOR_REV;
  int       MAX_PRIMER_BASES, MAX_READ_BASES, FIND_POLY_A, PARTIAL_ENDS;
  double    MAX_EDIT_DISTANCE, MIN_ALIGN_LEN;
}argopts_t;

typedef struct{
  char     *line;
  size_t    linelen;
  char     *seq;
  ssize_t   seq_size;
  char     *id;
  int       num;
}primer_t;

typedef struct{
  stats_t                stats;
  const files_t         *files;
  const read_t          *read;
  const argopts_t       *p;
  int                    thread_num;
}globals_t;

#endif
