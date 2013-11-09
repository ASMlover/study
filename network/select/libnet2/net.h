#ifndef __NET_HEADER_H__
#define __NET_HEADER_H__

enum NetType {
  kNetTypeInval = -1, 
  kNetTypeError = -1, 
};

enum EventType {
  kEventTypeUnknown = 0, 
  kEventTypeRead    = 1, 
  kEventTypeWrite   = 2, 
};


class NetLibrary {
  static bool loaded_;

  NetLibrary(const NetLibrary&);
  NetLibrary& operator =(const NetLibrary&);
public:
    explicit NetLibrary(void);
    ~NetLibrary(void);

    static NetLibrary& Singleton(void);

    bool Init(void);
    void Destroy(void);
};


#endif  //! __NET_HEADER_H__
