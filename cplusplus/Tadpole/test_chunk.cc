#include "harness.hh"
#include "chunk.hh"

TADPOLE_TEST(TadpoleChunk) {
  tadpole::Chunk c;

  // 67 + 45
  c.write_constant(45, 0);
  c.write_constant(67, 0);
  c.write(tadpole::Code::ADD, 0);

  // 89.9 * 56
  c.write_constant(56, 1);
  c.write_constant(89.9, 1);
  c.write(tadpole::Code::MUL, 1);

  // (34 - 67 + 121) / 0.89
  c.write_constant(0.89, 2);
  c.write_constant(121, 2);
  c.write_constant(67, 2);
  c.write_constant(34, 2);
  c.write(tadpole::Code::SUB, 2);
  c.write(tadpole::Code::ADD, 2);
  c.write(tadpole::Code::MUL, 2);

  // 1 + 3 * 45 / 12.4 - 78
  c.write_constant(78, 3);
  c.write_constant(12.4, 3);
  c.write_constant(45, 3);
  c.write_constant(3, 3);
  c.write(tadpole::Code::MUL, 3);
  c.write(tadpole::Code::DIV, 3);
  c.write_constant(1, 3);
  c.write(tadpole::Code::ADD, 3);
  c.write(tadpole::Code::SUB, 3);

  // 2 * (3 + 5) / (6 - 1)
  c.write_constant(1, 4);
  c.write_constant(6, 4);
  c.write(tadpole::Code::SUB, 4);
  c.write_constant(5, 4);
  c.write_constant(3, 4);
  c.write(tadpole::Code::ADD, 4);
  c.write_constant(2, 4);
  c.write(tadpole::Code::MUL, 4);
  c.write(tadpole::Code::DIV, 4);

  c.write(tadpole::Code::RETURN, 5);

  c.dis("TadpoleChunk");
}