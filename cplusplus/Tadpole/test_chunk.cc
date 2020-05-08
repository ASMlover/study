#include "harness.hh"
#include "chunk.hh"

TADPOLE_TEST(TadpoleChunk) {
  tadpole::Chunk c;

  c.write(tadpole::Code::CONSTANT, 0);
  c.write(c.add_constant(45), 0);
  c.write(tadpole::Code::CONSTANT, 0);
  c.write(c.add_constant(67), 0);

  c.write(tadpole::Code::RETURN, 1);

  c.dis("Tadpole.Chunk");
}