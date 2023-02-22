#pragma comment(lib, "Ws2_32.lib")
#include <stdlib.h>
#include <string.h>
#include "CircularBuffer.h"
#define BUFFER_SIZE 1024

struct circular_buffer
{
	char buffer[30][BUFFER_SIZE];
	int push;
	int pop;
	int push_count;
	int pop_count;
};

extern struct circular_buffer* cb = NULL;

int circularBufferPush(const char* data)
{
	if (cb == NULL) {
		cb = (struct circular_buffer*)malloc(sizeof(struct circular_buffer));

		cb->push = 0;
		cb->pop = 0;
		cb->push_count = 0;
		cb->pop_count = 0;
	}
	if (cb->push == 30)
	{
		cb->push = 0;
	}

	strcpy_s(cb->buffer[cb->push], data);
	cb->push++;
	cb->push_count++;

	return cb->push_count;
}

const char* circularBufferPop()
{
	if (cb == NULL) {
		return "";
	}

	if (cb->pop_count >= cb->push_count)
	{
		return "";
	}

	const char* data = cb->buffer[cb->pop];
	cb->pop++;
	cb->pop_count++;

	if (cb->pop == 30)
	{
		cb->pop = 0;
	}

	return data;
}

bool bufferCheck() {
	if (cb == NULL) {
		cb = (struct circular_buffer*)malloc(sizeof(struct circular_buffer));
		cb->push = 0;
		cb->pop = 0;
		cb->push_count = 0;
		cb->pop_count = 0;
	}

	if (cb->pop_count < cb->push_count) {
		return true;
	}
	else {
		return false;
	}
}