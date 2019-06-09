
// no trailing newline
{
  io.write("ok") }.call // expect: ok

// trailing newline
{
  io.write("ok") // expect: ok
}.call

// muiltiple expressions
{
  io.write("1") // expect: 1
  io.write("2") // expect: 2
}.call

// extra newlines
{

  io.write("1") // expect: 1

  io.write("2") // expect: 2


}.call
