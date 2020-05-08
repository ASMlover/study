#include <iostream>
#include "chunk.hh"

namespace tadpole {

inline sz_t dis_constant(Chunk* chunk, const char* msg, sz_t i) noexcept {
  auto c = chunk->get_code(i + 1);
  std::fprintf(stdout, "%-16s %4d ", msg, c);
  std::cout << "`" << chunk->get_constant(c) << "`" << std::endl;
  return i + 2;
}

inline sz_t dis_simple(Chunk* chunk, const char* msg, sz_t i) noexcept {
  std::fprintf(stdout, "%-16s\n", msg);
  return i + 1;
}

void Chunk::dis(strv_t msg) {
  std::cout << "========= [" << msg << "] =========" << std::endl;
  for (sz_t offset = 0; offset < codes_.size();)
    offset = dis_code(offset);
}

sz_t Chunk::dis_code(sz_t offset) {
  std::fprintf(stdout, "%04d ", as_type<int>(offset));
  if (offset > 0 && lines_[offset] == lines_[offset - 1])
    std::fprintf(stdout, "   | ");
  else
    std::fprintf(stdout, "%4d ", lines_[offset]);

  switch (auto c = as_type<Code>(codes_[offset])) {
  case Code::CONSTANT: return dis_constant(this, "CONSTANT", offset);
  case Code::NIL: return dis_simple(this, "NIL", offset);
  case Code::FALSE: return dis_simple(this, "FALSE", offset);
  case Code::TRUE: return dis_simple(this, "TRUE", offset);
  case Code::POP: return dis_simple(this, "POP", offset);

  case Code::RETURN: return dis_simple(this, "RETURN", offset);
  default: std::cerr << "<Invalid Code>" << std::endl; break;
  }
  return offset + 1;
}

}