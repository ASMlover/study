#include <stdio.h>
#include <stdlib.h>
#include "array.h"


struct array_s {
  int size;
  element_t* elements;
};


#define ASSERT_ARRAY(a) {\
  if (NULL == (a)) {\
    fprintf(stderr, "array invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_ARRAY_UNDERFLOW(i) {\
  if ((i) < 0) {\
    fprintf(stderr, "array underflow ...");\
    exit(0);\
  }\
}
#define ASSERT_ARRAY_OVERFLOW(a, i) {\
  if ((i) >= (a)->size) {\
    fprintf(stderr, "array overflow ...");\
    exit(0);\
  }\
}




array_t 
array_create(int size) 
{
  array_t a = (array_t)malloc(sizeof(*a));
  ASSERT_ARRAY(a);

  a->size = size;
  a->elements = (element_t*)malloc(a->size * sizeof(element_t));
  if (NULL == a->elements) {
    fprintf(stderr, "create array failed ...");
    exit(0);
  }

  return a;
}

void 
array_delete(array_t* a) 
{
  if (NULL != *a) {
    if (NULL != (*a)->elements) 
      free((*a)->elements);
    free(*a);
    *a = NULL;
  }
}

int 
array_empty(array_t a) 
{
  ASSERT_ARRAY(a);

  return (0 == a->size);
}

int 
array_size(array_t a) 
{
  ASSERT_ARRAY(a);

  return (a->size);
}

void 
array_set(array_t a, int i, element_t e)
{
  ASSERT_ARRAY(a);
  ASSERT_ARRAY_UNDERFLOW(i);
  ASSERT_ARRAY_OVERFLOW(a, i);

  a->elements[i] = e;
}

element_t 
array_get(array_t a, int i) 
{
  ASSERT_ARRAY(a);
  ASSERT_ARRAY_UNDERFLOW(i);
  ASSERT_ARRAY_OVERFLOW(a, i);

  return a->elements[i];
}

element_t* 
array_data(array_t a)
{
  ASSERT_ARRAY(a);

  return a->elements;
}

void 
array_traverse(array_t a, void (*visit)(element_t))
{
  int i;
  ASSERT_ARRAY(a);

  for (i = 0; i < a->size; ++i)
    visit(a->elements[i]);
}
