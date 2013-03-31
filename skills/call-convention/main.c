// MSVC cl /FAs main.asm main.c
// GCC gcc -S -o main.s main.c

static int __cdecl 
add(int a, int b) 
{
  return a + b;
}

static int __stdcall 
sub(int a, int b)
{
  return a - b;
}
  
int 
main(int argc, char* argv[])
{
  add(11, 22);
  sub(11, 22);

  return 0;
}
