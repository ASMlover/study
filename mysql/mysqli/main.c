#include <stdio.h>
#include <stdlib.h>
#include "mysqli.h"

int 
main(int argc, char* argv[], char* envp[])
{
  void* conn = mysqli_connect("192.168.120.30", 3306, "root", "123", NULL, "gbk");

  if (0 == mysqli_execute(conn, "select platid, platname from oms.t_plat;"))
  {
    char** row;

    int      fields = mysqli_fields(conn);
    uint64_t rows   = mysqli_rows(conn);
    fprintf(stdout, "fields = %d, rows = %I64u\n", fields, rows);
    while (NULL != (row = mysqli_row(conn)))
      fprintf(stdout, "{platid=>%d, platname=>%s}\n", atoi(row[0]), row[1]);
  }

  mysqli_close(&conn);
  return 0;
}
