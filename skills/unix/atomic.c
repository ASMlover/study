#include <stdio.h>

typedef struct {
  volatile int counter;
} atomic_t;


static inline void 
atomic_inc(atomic_t* v) 
{
  __asm__ __volatile__ (
    "incl %0"
    : "+m" (v->counter));
}

static inline void 
atomic_dec(atomic_t* v) 
{
  __asm__ __volatile__ (
    "decl %0"
    : "+m" (v->counter));
}

int 
main(int argc, char* argv[])
{
  int l = 0;

  atomic_inc((atomic_t*)&l);
  fprintf(stdout, "after atomic_inc : %d\n", l);
  atomic_dec((atomic_t*)&l);
  fprintf(stdout, "after atomic_dec : %d\n", l);

  return 0;
}
