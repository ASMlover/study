#ifndef __UNCOPYABLE_HEADER_HPP__
#define __UNCOPYABLE_HEADER_HPP__

class UnCopyable {
  UnCopyable(const UnCopyable&);
  UnCopyable& operator=(const UnCopyable&);
protected:
  UnCopyable(void) {}
  ~UnCopyable(void) {}
};

#endif  // __UNCOPYABLE_HEADER_HPP__
