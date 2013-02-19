#include <zmq.h>
#include <stdio.h>
#if (defined(_WIN32) || defined(_WIN64))
  #include <windows.h>
  #define __sleep     Sleep
#else
  #include <unistd.h>
  #define __sleep(m)  usleep(m * 1000)
#endif


int 
main(int argc, char* argv[])
{
  int i;
  void* ctx = zmq_ctx_new();
  void* s = zmq_socket(ctx, ZMQ_REQ);
  zmq_connect(s, "tcp://localhost:5555");

  for (i = 0; i < 10; ++i) {
    zmq_msg_t req, rep;

    zmq_msg_init_size(&req, 128);
    sprintf((char*)zmq_msg_data(&req), 
      "Hello, world! current count[%d]", i);
    zmq_msg_send(&req, s, 0);
    zmq_msg_close(&req);

    zmq_msg_init(&rep);
    zmq_msg_recv(&rep, s, 0);
    fprintf(stdout, "reply message : %s\n", (char*)zmq_msg_data(&rep));
    zmq_msg_close(&rep);

    __sleep(100);
  }

  zmq_close(s);
  zmq_ctx_destroy(ctx);

  return 0;
}
