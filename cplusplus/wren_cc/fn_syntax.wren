
// no trailing newline
fn {
  IO.write("ok") }.call // expect: ok

// trailing newline
fn {
  IO.write("ok") // expect: ok
}.call

// muiltiple expressIO.s
fn {
  IO.write("1") // expect: 1
  IO.write("2") // expect: 2
}.call

// extra newlines
fn {

  IO.write("1") // expect: 1

  IO.write("2") // expect: 2


}.call
