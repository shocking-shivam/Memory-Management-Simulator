#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stddef.h>

/* Application-facing malloc/free wrapper */
void *my_malloc(size_t size);
void my_free(void *ptr);

#endif /* MY_MALLOC_H */
