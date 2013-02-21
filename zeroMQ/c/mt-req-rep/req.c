#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zmq.h> 


int 
main(int argc, char* argv[])
{
  void* ctx = zmq_ctx_new();
  void* req = zmq_socket(ctx, ZMQ_REQ);
  zmq_connect(req, "tcp://localhost:5555");

  do {
    int i, num_req;
    char buf[128];

    if (argc < 2) {
      fprintf(stderr, "arguments error ...\n");
      break;
    }
    num_req = atoi(argv[1]);

    srand((unsigned int)time(0));
    for (i = 0; i < num_req; ++i) {
      sprintf(buf, "request id {%d}", rand() % 1000);
      zmq_send(req, buf, strlen(buf), 0);
      fprintf(stdout, "send request : %s\n", buf);

      memset(buf, 0, sizeof(buf));
      zmq_recv(req, buf, sizeof(buf), 0);
      fprintf(stdout, "recevied from reply : %s\n", buf);
    }
  } while (0);

  zmq_close(req);
  zmq_ctx_destroy(ctx);

  return 0;
}
