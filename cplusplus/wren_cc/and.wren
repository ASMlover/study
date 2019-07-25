
// return the first non-true argument
IO.print(false && 1) // expect: false
IO.print(true && 1) // expect: 1
IO.print(1 && 2 && false) // expect: false

// return the last argument if all are true
IO.print(1 && true) // expect: true
IO.print(1 && 2 && 3) // expect: 3

// short-circuit at the first false argument
IO.print(true) && // expect: true
  IO.print(false) && // expect: false
  IO.print(false)

// swallow a trailing newline
IO.print(true &&
  true) // expect: true

// only nil and false is falsely
IO.print(0 && true) // expect: true
IO.print(nil && true) // expect: nil
IO.print("" && true) // expect: true
IO.print(false && true) // expect: false
