
// no trailing newline
fn {
  IO.print("ok") }.call // expect: ok

// trailing newline
fn {
  IO.print("ok") // expect: ok
}.call

// muiltiple expressIO.s
fn {
  IO.print("1") // expect: 1
  IO.print("2") // expect: 2
}.call

// extra newlines
fn {

  IO.print("1") // expect: 1

  IO.print("2") // expect: 2


}.call
