#include <zmq.h>


int 
main(int argc, char* argv[])
{
  void* ctx = zmq_ctx_new();

  void* s = zmq_socket(ctx, ZMQ_REP);
  zmq_bind(s, "tcp://*:5555");
  fprintf(stdout, "rep server init success ...\n");
  while (1) {
    zmq_msg_t msg;

    zmq_msg_init(&msg);
    zmq_msg_recv(&msg, s, 0);
    zmq_msg_send(&msg, s, 0);
    zmq_msg_close(&msg);
  }
  zmq_close(s);

  zmq_ctx_destroy(ctx);

  return 0;
}
