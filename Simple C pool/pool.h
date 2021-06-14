#ifndef __POOL_H__
#define __POOL_H__

typedef struct Elem_ {
	void* mem;
	int size;
	int is_empty;
}Elem;

typedef struct Pool_ {
	Elem* elem;
	int size;
}Pool;

void init_pool(Pool** pool, int size, int elem_size_in_bytes);
void del_pool(Pool* pool);
int add_element(Pool* pool, void* element);
void* get_elemnt(Pool* pool, int num);
void* get_last_elemnt(Pool* pool);

#endif

