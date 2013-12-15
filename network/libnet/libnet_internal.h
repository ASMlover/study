#ifndef __LIBNET_INTERNAL_HEADER_H__
#define __LIBNET_INTERNAL_HEADER_H__

#include "libnet.h"

enum NetType {
  kNetTypeInval = -1, 
  kNetTypeError = -1,
};

enum EventType {
  kEventTypeUnknown = 0x00, 
  kEventTypeRead    = 0x01, 
  kEventTypeWrite   = 0x02, 
};


class Connector;
struct Poller;
struct Dispatcher {
  virtual ~Dispatcher(void) {}
  virtual bool DispatchReader(Poller* poller, Connector* conn) = 0;
  virtual bool DispatchWriter(Poller* poller, Connector* conn) = 0;
};

struct Poller {
  virtual ~Poller(void) {}
  virtual bool Insert(Connector* conn) = 0;
  virtual void Remove(Connector* conn) = 0;
  virtual bool AddEvent(Connector* conn, int ev) = 0;
  virtual bool DelEvent(Connector* conn, int ev) = 0;
  virtual bool Dispatch(Dispatcher* dispatcher, int millitm) = 0;
};

#endif  //! __LIBNET_INTERNAL_HEADER_H__
