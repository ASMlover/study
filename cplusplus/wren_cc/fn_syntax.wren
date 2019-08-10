
// no trailing newline
new Function {
  IO.print("ok") }.call // expect: ok

// trailing newline
new Function {
  IO.print("ok") // expect: ok
}.call

// muiltiple expressIO.s
new Function {
  IO.print("1") // expect: 1
  IO.print("2") // expect: 2
}.call

// extra newlines
new Function {

  IO.print("1") // expect: 1

  IO.print("2") // expect: 2


}.call
