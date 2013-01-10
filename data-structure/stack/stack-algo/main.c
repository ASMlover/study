#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


/*
 * 解析算术表达式, 获取每一个数(不管是整数还是浮点数)
 */
int 
main(int argc, char* argv[])
{
  char* exp = "9 + (3 - 0.1) * 2.1 + 12 / 1.2 - 4.5";

  while (*exp) {
    if (isspace(*exp)) 
      ++exp;
    else if ('+' == *exp || '-' == *exp 
      || '*' == *exp || '/' == *exp 
      || '(' == *exp || ')' == *exp) {
      ++exp;
    }
    else {
      char buf[32] = {0};
      int i = 0;
      while (*exp) {
        if (isdigit(*exp) || '.' == *exp)
          buf[i++] = *exp++;
        else {
          if (0 != i) 
            fprintf(stdout, "%f ", atof(buf));
          break;
        }
      }
    }
  }
  fprintf(stdout, "\n");

  return 0;
}
