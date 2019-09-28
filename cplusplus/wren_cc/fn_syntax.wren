
// single expression body
Function.new { IO.print("ok") }.call() // expect: ok

// curly body
Function.new {
  IO.print("ok") // expect: ok
}.call()

// muiltiple statements
Function.new {
  IO.print("1") // expect: 1
  IO.print("2") // expect: 2
}.call()

// extra newlines
Function.new {

  IO.print("1") // expect: 1

  IO.print("2") // expect: 2


}.call()
