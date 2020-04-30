#include <Core/Harness.hh>
#include <Tadpole/Chunk.hh>

_MEVO_TEST(TadpoleChunk, _mevo::FakeTester) {
  _mevo::tadpole::Chunk c;

  // 45 + 67
  c.write(_mevo::tadpole::Code::CONSTANT, 0);
  c.write(c.add_constant(45), 0);
  c.write(_mevo::tadpole::Code::CONSTANT, 0);
  c.write(c.add_constant(67), 0);
  c.write(_mevo::tadpole::Code::ADD, 0);

  c.write(_mevo::tadpole::Code::RETURN, 1);

  c.dis("Tadpole.Chunk");
}