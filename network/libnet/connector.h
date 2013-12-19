#ifndef __CONNECTOR_HEADER_H__
#define __CONNECTOR_HEADER_H__

#include "locker.h"
#include "buffer.h"

class Connector : public Socket {
  uint32_t events_;
  SpinLock spinlock_;
  Buffer rbuf_;
  Buffer wbuf_;

  Connector(const Connector&);
  Connector& operator =(const Connector&);
public:
  explicit Connector(void);
  ~Connector(void);

  inline bool SetReadBuffer(uint32_t bytes)
  {
    return rbuf_.Init(bytes);
  }

  inline bool SetWriteBuffer(uint32_t bytes)
  {
    return wbuf_.Init(bytes);
  }

  inline uint32_t events(void) const 
  {
    return events_;
  }

  inline void set_events(uint32_t events)
  {
    events_ = events;
  }
public:
  int Read(uint32_t bytes, char* buffer);
  int Write(const char* buffer, uint32_t bytes);

  bool WriteBufferEmpty(void);

  int DealWithAsyncRead(void);
  int DealWithAsyncWrite(void);
};

#endif  //! __CONNECTOR_HEADER_H__
