
// return the first non-true argument
IO.write(false && 1) // expect: false
IO.write(true && 1) // expect: 1
IO.write(1 && 2 && false) // expect: false

// return the last argument if all are true
IO.write(1 && true) // expect: true
IO.write(1 && 2 && 3) // expect: 3

// short-circuit at the first false argument
IO.write(true) && // expect: true
  IO.write(false) && // expect: false
  IO.write(false)

// swallow a trailing newline
IO.write(true &&
  true) // expect: true

// only nil and false is falsely
IO.write(0 && true) // expect: true
IO.write(nil && true) // expect: nil
IO.write("" && true) // expect: true
IO.write(false && true) // expect: false
