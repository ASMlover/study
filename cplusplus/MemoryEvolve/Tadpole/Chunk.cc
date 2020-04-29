#include <iostream>
#include <Tadpole/Chunk.hh>

namespace _mevo::tadpole {

void Chunk::dis(const str_t& s) {
  std::cout << "========= [" << s << "] =========" << std::endl;
  for (int i = 0; i < count();)
    i = dis_code(i);
}

int Chunk::dis_code(int i) {
  return i;
}

}