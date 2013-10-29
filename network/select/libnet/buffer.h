#ifndef __BUFFER_HEADER_H__
#define __BUFFER_HEADER_H__

class Buffer {
  char* buffer_;
  int   storage_;
  int   length_;

  Buffer(const Buffer&);
  Buffer& operator =(const Buffer&);
public:
  enum { kDefaultStorage = 16 * 1024 };
  explicit Buffer(void);
  ~Buffer(void);

  inline const char* buffer(void) const 
  {
    return buffer_;
  }

  inline int length(void) const 
  {
    return length_;
  }

  inline char* free_buffer(void) const 
  {
    return buffer_ + length_;
  }

  inline int free_Length(void) const 
  {
    return storage_ - length_;
  }
public:
  bool Init(int storage = kDefaultStorage);
  void Destroy(void);

  int Put(const char* buffer, int bytes);
  int Get(int bytes, char* buffer);
  int Increment(int bytes);
  int Decrement(int bytes);

  bool Regrow(void);
};

#endif  //! __BUFFER_HEADER_H__
