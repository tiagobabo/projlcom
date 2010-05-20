#ifndef _GQUEUE_H_
#define _GQUEUE_H_
#include "stdlib.h"
#include "utypes.h"
#include "string.h"

typedef struct {
	void *ptr;
	int in, out, count, size, ele_size;
} GQueue;


GQueue * newGQueue( int n_ele, int size_ele );
void deleteGQueue( GQueue *q );
int putGQueue( GQueue *q, void *ele );
void * getGqueue( GQueue *q );
int isEmptyGQueue( GQueue *q );
int isFullGQueue( GQueue *q );

#endif
