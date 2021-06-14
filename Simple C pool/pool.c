#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <synchapi.h>
#include "pool.h"

CRITICAL_SECTION lpCriticalSection;

void init_pool(Pool **pool, int size, int elem_size_in_bytes) {
	*pool = (Pool*)malloc(sizeof(Pool));
	(*pool)->elem = (Elem*)malloc(size * sizeof(Elem));
	for (int i = 0; i < size; i++) {
		(*pool)->elem[i].mem = malloc(elem_size_in_bytes);
		(*pool)->elem[i].size = elem_size_in_bytes;
		(*pool)->elem[i].is_empty = 1;
	}
	(*pool)->size = size;
	InitializeCriticalSection(&lpCriticalSection);
}

void del_pool(Pool* pool) {
	int size = pool->size;
	for (int i = 0; i < size; i++) {
		free(pool->elem[i].mem);
	}
	free(pool->elem);
	free(pool);
	DeleteCriticalSection(&lpCriticalSection);
}

int add_element(Pool* pool, void* element) {
	int i = 0;
	EnterCriticalSection(&lpCriticalSection);
	for (i = 0; i < pool->size; i++) {
		if (pool->elem[i].is_empty) {
			memcpy(pool->elem[i].mem, element, pool->elem[i].size);
			pool->elem[i].is_empty = 0;
			break;
		}
	}
	LeaveCriticalSection(&lpCriticalSection);
	return (i < pool->size);
}

void* get_elemnt(Pool* pool, int num) {
	EnterCriticalSection(&lpCriticalSection);
	if ((num < pool->size) && (!pool->elem[num].is_empty)) {
		pool->elem[num].is_empty = 1;
		LeaveCriticalSection(&lpCriticalSection);
		return pool->elem[num].mem;
	}
	LeaveCriticalSection(&lpCriticalSection);
	return NULL;
}

void* get_last_elemnt(Pool* pool) {
	int i = pool->size-1;
	void* ret = NULL;
	EnterCriticalSection(&lpCriticalSection);
	for (; i >= 0; i--) {
		if (!pool->elem[i].is_empty) {
			pool->elem[i].is_empty = 1;
			ret = pool->elem[i].mem;
			break;
		}
	}
	LeaveCriticalSection(&lpCriticalSection);
	return ret;
}
