#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#ifdef PARALLEL
#include <pthread.h>
#endif
#include "DB.h"
#include "args.h"
#include "process.h"
#include "align.h"
#include "debug.h"
#include "SequencingChannel.h"

static void *process_thread(void *arg);

int main (int argc, char *argv[]){
  globals_t         *g;
  wmk_config_t      *wcfg;
  files_t            files;
  argopts_t          p;
  programArguments   pArgs;
  int                i;
  #ifdef PARALLEL
  pthread_t         *threads;
  #endif

  arg_parse(argc, argv, &p);
  wmk_init(&pArgs);
  if(p.VERBOSE > 4) arg_print(&p);

  g = (globals_t *)Malloc(p.NUM_THREADS * sizeof(globals_t),"Global(ish) variables.");
  wcfg = (wmk_config_t *)Malloc(p.NUM_THREADS * sizeof(wmk_config_t),"Watermark config variables.");

  #ifdef PARALLEL
  threads = (pthread_t *)Malloc(p.NUM_THREADS * sizeof(pthread_t),"Thread variables.");
  #endif

  arg_post(&p, &files);
  load_primers(&p);
  sort_primers(&p);
  #ifdef DEBUG
  print_primers(&p);
  #endif

  for(i = 0; i < p.NUM_THREADS; i++){
    wmk_config_init(&pArgs, wcfg+i);
    g[i].p = &p;
    g[i].pArgs = &pArgs;
    g[i].wcfg = wcfg+i;
    g[i].files = &files;
    g[i].thread_num = i;
    #ifdef PARALLEL
    pthread_create(threads+i,NULL,process_thread,(void *) (g+i));
    #endif
  }

  #ifdef PARALLEL
  for(i = 0; i < p.NUM_THREADS; i++){
    pthread_join(threads[i],NULL);
    //if(i) collect_stats(g,g+i);
  }
  #else
  process_file(g);
  #endif

  DEBUG_PRINT("Closing input and output files.\n");
  fclose(files.ifp);
  for(i = 0; i < 512; i++){
    if(files.ofp[i]) fclose(files.ofp[i]);
  }
  DEBUG_PRINT("Freeing file name buffer.\n");
  free(files.ofnames[0]);
  if(p.BC_WHITELIST_FILE) free(p.TRUE_BC[0]);

  for(i = 0; i < p.NUM_THREADS; i++){
    wmk_config_free(wcfg+i);
  }
  free(wcfg);

  wmk_free();

  clean_primers(&p);
  free(g);
  #ifdef PARALLEL
  free(threads);
  #endif

  return (0);
}

static void *process_thread(void *arg){
  return process_file((globals_t*) arg);
}

