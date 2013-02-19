#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static char s_buf[128];

static void 
publish_message(void* s, int idx)
{
  char sub[8];
  sprintf(sub, "%d", idx);
  zmq_send(s, sub, 1, ZMQ_SNDMORE);

  sprintf(s_buf, "Publish message about `%d`", idx);
  zmq_send(s, s_buf, strlen(s_buf), 0);
}


int 
main(int argc, char* argv[])
{
  int num = atoi(argv[1]);
  void* ctx = zmq_ctx_new();
  void* s = zmq_socket(ctx, ZMQ_PUB);
  zmq_bind(s, "tcp://*:5555");

  fprintf(stdout, "publish server init success ...\n");
  while (1) {
    int i;
    for (i = 0; i < num; ++i) 
      publish_message(s, i);

    sleep(1);
  }

  zmq_close(s);
  zmq_ctx_destroy(ctx);

  return 0;
}
