#ifndef __MY_MALLOC__
#define __MY_MALLOC__

#include <stdlib.h>

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))


void *my_malloc(size_t size);

void my_free(void *ptr);

void *my_realloc(void *ptr, size_t size);

void *my_calloc(size_t nbr_elements, size_t element_size);

#endif