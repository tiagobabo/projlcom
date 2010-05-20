#include "gqueue.h"

GQueue * newGQueue( int n_ele, int ele_size )
{
   GQueue *q = (GQueue *) malloc( sizeof(GQueue) );
   q->ptr = malloc( n_ele * ele_size );
   q->in = q->out = q->count = 0;
   q->size = n_ele; q->ele_size = ele_size;
   return q;
}


void deleteGQueue( GQueue* q )
{
   free(q->ptr); free(q);
}


int putGQueue( GQueue *q, void *ele )
{
   if( isFullGQueue(q) )
    return false;

   memcpy( q->ptr + q->in, ele, q->ele_size );

   q->in = ( q->in + q->ele_size );

   if( q->in >= q->size * q->ele_size ){ q->in = 0; }
   q->count++;

   return true;
}


void * getGqueue( GQueue *q )
{
   if( isEmptyGQueue(q) ) return NULL;

   void * ret = q->ptr + q->out;
   q->out = ( q->out + q->ele_size );

   if( q->out >= q->size * q->ele_size ) q->out = 0;
   q->count--;

   return ret;
}


int isEmptyGQueue( GQueue *q )
{
   return q->count == 0;
}


int isFullGQueue( GQueue *q )
{
   return q->count == q->size;
}
