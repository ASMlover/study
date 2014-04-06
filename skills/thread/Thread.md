# **THREAD**
***


## **杀死线程**
    建议不要杀死线程, 因为杀死线程容易产生各种各样的问题, 最典型的问题就是内
    存泄露;
    
    Windows下:
      TerminateThread(thread_handle, 0);
      CloseHandle(thread_handle);
    Linux下:
      pthread_cancel(thread_id);
