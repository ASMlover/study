#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "array.h"


static void 
visit_double(element_t e) 
{
  fprintf(stdout, "the array element value is : %lf\n", *(double*)e);
}

static void 
array_test(array_t a, 
  void (*set)(array_t, int, element_t), 
  element_t (*get)(array_t, int), 
  int rand_n, 
  double rand_d)
{
  int i;
  double* d;
  double** dd;

  srand(time(0));
  for (i = 0; i < 10; ++i) {
    d = (double*)malloc(sizeof(*d));
    *d = rand() % rand_n *  rand_d;
    set(a, i, d);

    fprintf(stdout, "set array element {i=>%d, v=>%lf}\n", i, *d);
  }
  fprintf(stdout, "array {size=>%d, empty=>%d}\n", 
    array_size(a), array_empty(a));

  fprintf(stdout, "\nshow array elements : \n");
  array_traverse(a, visit_double);

  fprintf(stdout, "\nshow array elements : \n");
  dd = (double**)array_data(a);
  for (i = 0; i < 10; ++i) 
    fprintf(stdout, "array element is {i=>%d, v=>%lf}\n", i, *dd[i]);

  fprintf(stdout, "\nshow array elements : \n");
  for (i = 0; i < 10; ++i) {
    d = (double*)array_get(a, i);
    fprintf(stdout, "array element is {i=>%d, v=>%lf}\n", i, *d);
    free(d);
  }
}


int 
main(int argc, char* argv[])
{
  array_t a = array_create(10);

  array_test(a, array_set, array_get, 45463, 0.98792);

  array_delete(&a);
  return 0;
}
