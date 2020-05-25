//    Filename: header.h
//
//    Description: Some utilites for the TinySearchEngine engine project - MACROs for 
//              general memory allocation and initialization and some 
//              program exceptions processing
//


// Note, the header check below makes sure you do not include a header file twice. Use it.

#ifndef _HEADER_H_
#define _HEADER_H_

#define min(x,y)   ((x)>(y))?(y):(x)

// Print  s together with the source file name and the current line number.
#define LOG(s)  printf("[%s:%d]%s\n", __FILE__, __LINE__, s)

// malloc a new data structure t

#define NEW(t) malloc(sizeof(t))

// Check whether  s is NULL or not. Quit this program if it is NULL.
#define MYASSERT(s)  if (!(s))   {                                      \
    printf("General Assert Error at %s:line%d\n", __FILE__, __LINE__);  \
    exit(-1); \
  }


// Check whether s is NULL or not on a memory allocation. Quit this program if it is NULL.
#define MALLOC_CHECK(s)  if ((s) == NULL)   {                     \
    printf("No enough memory at %s:line%d ", __FILE__, __LINE__); \
    perror(":");                                                  \
    exit(-1); \
  }

// Set memory space starts at pointer \a n of size \a m to zero. 
#define BZERO(n,m)  memset(n, 0, m)

#endif

