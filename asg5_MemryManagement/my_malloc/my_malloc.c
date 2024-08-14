#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include "my_malloc.h"

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

typedef struct list_t list_t;
struct list_t {
	size_t 		size;
	unsigned 	in_use:1;	/* if the block is used or not */
	list_t* 	next;		/* next available block. */
};

#define HEADER_SIZE (ALIGN(sizeof(list_t)))

static list_t* allocate_space(list_t*, size_t);
static list_t* find_block(list_t**, size_t);

static list_t* base = NULL;

static list_t* allocate_space(list_t* last, size_t size)
{

	list_t* header = sbrk(0);
	void* data = sbrk(ALIGN(size + HEADER_SIZE));
    // return error
	if (data == (void*) - 1) {
		return NULL;
	}
    // data: hold previous program break
	header->size = size;
	header->next = NULL;
	header->in_use = 1;
	if (last) {
		last->next = data;
	}

	return header;
}

static list_t* find_block(list_t** last, size_t size)
{
	list_t* current = base;
	while (current) {
		if (!current->in_use && current->size >= size)
			return current;

		*last = current;
		current = current->next;
	}
	return current;
}

void *my_malloc(size_t size)
{
	if (size <= 0) {
		return NULL;
	}

	list_t* last = base;
	list_t* r = find_block(&last, size);

	if (r == NULL) {
		r = allocate_space(last, size);
		if (r == NULL)
			return NULL;

		if (base == NULL)
			base = r;
	} else {
		r->in_use = 1;
	}

	return (r + 1);

}

void my_free(void *ptr) {
	
	if (!ptr)
		return;
	list_t* block = ((list_t*) ptr) - 1;
	assert(block->in_use == 1);
	block->in_use = 0;
}

void *my_calloc(size_t nbr_elements, size_t element_size) {
	size_t size = nbr_elements * element_size;
	void* ptr = my_malloc(size);
	if (ptr == NULL)
		return NULL;
	memset(ptr, 0, size);
	return ptr;
}

void *my_realloc(void *ptr, size_t size) {

	if (!ptr) {
		return my_malloc(size);
	}

	list_t* block = ((list_t*) ptr) - 1;
	if (block->size >= size) {
		return ptr;
	}

	void* new_ptr = my_malloc(size);
	if (!new_ptr) {
		return NULL;
	}

	memcpy(new_ptr, ptr, block->size);
	my_free(ptr);
	return new_ptr;
}