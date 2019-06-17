
// no trailing newline
fn {
  io.write("ok") }.call // expect: ok

// trailing newline
fn {
  io.write("ok") // expect: ok
}.call

// muiltiple expressions
fn {
  io.write("1") // expect: 1
  io.write("2") // expect: 2
}.call

// extra newlines
fn {

  io.write("1") // expect: 1

  io.write("2") // expect: 2


}.call
