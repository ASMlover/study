#ifndef __ADDRESS_HEADER_H__
#define __ADDRESS_HEADER_H__

struct sockaddr_in;
class Address {
  enum {kDefaultAddressLen = 16};
  char ip_[kDefaultAddressLen];
  uint16_t port_;

  Address(const Address&);
  Address& operator =(const Address&);
public:
  explicit Address(void);
  ~Address(void);

  inline const char* ip(void) const 
  {
    return ip_;
  }

  inline uint16_t port(void) const 
  {
    return port_;
  }
public:
  void Attach(const sockaddr_in* addr);
  void Detach(void);
};

#endif  //! __ADDRESS_HEADER_H__
