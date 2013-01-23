#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "btree.h"


#define ARRAY_N   (10)

static void 
search_btree_test(btree_t bt, int rand_n, double rand_d) 
{
  int i;
  double* d;
  int key_array[ARRAY_N];

  fprintf(stdout, "search binary tree testing ...\n");
  srand(time(0));
  for (i = 0; i < ARRAY_N; ++i) {
    d = (double*)malloc(sizeof(*d));
    key_array[i] = rand() % rand_n;
    *d = key_array[i] * rand_d;
    fprintf(stdout, "insert => {key=>%d, val=>%lf}\n", key_array[i], *d);
    btree_insert(bt, key_array[i], d);
  }
  fprintf(stdout, "\nsearch binary tree size : %d\n", btree_size(bt));
  for (i = 0; i < ARRAY_N; ++i) {
    d = btree_remove(bt, key_array[i]);
    fprintf(stdout, "the remove {key=>%d, val=>%lf}\n", key_array[i], *d);
    free(d);
  }
  fprintf(stdout, "\nsearch binary tree size : %d\n", btree_size(bt));
}


int 
main(int argc, char* argv[])
{
  btree_t bt = btree_create();

  search_btree_test(bt, 35466, 0.08982);

  btree_delete(&bt);
  return 0;
}
