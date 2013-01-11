#ifndef __ARRAY_HEADER_H__
#define __ARRAY_HEADER_H__ 

typedef struct array_s* array_t;
typedef void* element_t;


extern array_t array_create(int size);
extern void array_delete(array_t* a);

extern int array_empty(array_t a);
extern int array_size(array_t a);
extern void array_set(array_t a, int i, element_t e);
extern element_t array_get(array_t a, int i);
extern element_t* array_data(array_t a);
extern void array_traverse(array_t a, void (*visit)(element_t));

#endif  /* __ARRAY_HEADER_H__ */
