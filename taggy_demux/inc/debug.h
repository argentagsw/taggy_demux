#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef DEBUG
 #define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__); fflush(stderr)
#else
 #define DEBUG_PRINT(...) do{ } while ( 0 ) /* Don't do anything in release builds */
#endif

#endif

