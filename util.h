#ifndef util_h_
#define util_h_
#include "utypes.h"
#include "GQueue.h"

GQueue *mouse_queue;
//extern int all_data;
volatile Byte* arr;
volatile Byte b1, b2, b3;

typedef struct//estrutura para guardar os 3 bytes do rato
{
	Byte b1, b2, b3;
} MouseData;

void parse_mouse_event();

void process_mouse_event();

#endif
