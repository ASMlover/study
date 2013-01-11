#ifndef __ARRAY_HEADER_H__
#define __ARRAY_HEADER_H__ 

typedef struct array_s* array_t;
typedef void* element_t;

extern array_t array_create(int capacity);
extern void array_delete(array_t* a);

extern int array_empty(array_t a);
extern int array_size(array_t a);
extern int array_capacity(array_t a);
extern void array_clear(array_t a);
extern void array_push_front(array_t a, element_t e);
extern void array_push_back(array_t a, element_t e);
extern void array_insert(array_t a, int i, element_t e);
extern element_t array_pop_front(array_t a);
extern element_t array_pop_back(array_t a);
extern element_t array_erase(array_t a, int i);
extern void array_set(array_t a, int i, element_t e);
extern element_t array_get(array_t a, int i);

#endif  /* __ARRAY_HEADER_H__ */
