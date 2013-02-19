#include <zmq.h>
#include <stdio.h>


int 
main(int argc, char* argv[]) 
{
  void* ctx = zmq_ctx_new();
  void* s = zmq_socket(ctx, ZMQ_SUB);
  zmq_connect(s, "tcp://localhost:5555");
  zmq_setsockopt(s, ZMQ_SUBSCRIBE, argv[1], 1);

  while (1) {
    char address[8] = {0};
    char recvbuf[128] = {0};
    zmq_recv(s, address, 8, 0);
    zmq_recv(s, recvbuf, sizeof(recvbuf), 0);

    fprintf(stdout, "Get Publish message '%s' == <%s>\n", 
      recvbuf, address);
  }

  zmq_close(s);
  zmq_ctx_destroy(ctx);

  return 0;
}
