#include <stdio.h>
#include <math.h>
#include <pc.h>
#include "music.h"
#include "timer.h"
#include "queue.h"

void queueInit(Queue *q)
{
	q->size=10;
	q->cnt=0;
	q->in=0;
	q->out=0;
}

Bool queuePut(Queue *q, char c)
{
	if( q->cnt == q->size ) 
	{
	//void beep();
	//beep();
	return false; 
	}
	
	q->buf[ q->in ] = c;
	q->in = q->in == q->size-1 ? 0 : q->in+1;
	q->cnt++;
	return true;
}

int queueGet(Queue *q)
{
	disable_irq(1);
	if( q->cnt <= 0 ) { enable_irq(1); return -1; }
	
	q->cnt = q->cnt-1;
	char tmp = q->buf[ q->out ];
	q->out = q->out == q->size-1 ? 0 : q->out+1;
	enable_irq(1);
	return tmp;
}

Bool queueEmpty(Queue *q)
{ return q->cnt == 0 ? true : false; }

Bool queueFull(Queue *q)
{  return q->cnt == q->size ? true : false;  }
