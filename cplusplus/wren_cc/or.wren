
// return the first true argument
io.write(1 || true) // expect: 1
io.write(false || 1) // expect: 1
io.write(false || false || true) // expect: true

// return the last argument if all are false
io.write(false || false) // expect: false
io.write(false || nil || false) // expect: false

// short-circuit at the first true argument
io.write(false) || // expect: false
  io.write(true) || // expect: true
  io.write(true)

// swallow a trailing newline
io.write(true ||
  true) // expect: true

// only nil and false are falsely
io.write(0 || true) // expect: 0
io.write(nil || true) // expect: true
io.write(("" || true) == "") // expect: true
io.write(false || true) // expect: true
