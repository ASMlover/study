
// return the first non-true argument
io.write(false && 1) // expect: false
io.write(true && 1) // expect: 1
io.write(1 && 2 && false) // expect: false

// return the last argument if all are true
io.write(1 && true) // expect: true
io.write(1 && 2 && 3) // expect: 3

// short-circuit at the first false argument
io.write(true) && // expect: true
  io.write(false) && // expect: false
  io.write(false)

// swallow a trailing newline
io.write(true &&
  true) // expect: true

// only nil and false is falsely
io.write(0 && true) // expect: true
io.write(nil && true) // expect: nil
io.write("" && true) // expect: true
io.write(false && true) // expect: false
