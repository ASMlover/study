
// return the first true argument
IO.write(1 || true) // expect: 1
IO.write(false || 1) // expect: 1
IO.write(false || false || true) // expect: true

// return the last argument if all are false
IO.write(false || false) // expect: false
IO.write(false || nil || false) // expect: false

// short-circuit at the first true argument
IO.write(false) || // expect: false
  IO.write(true) || // expect: true
  IO.write(true)

// swallow a trailing newline
IO.write(true ||
  true) // expect: true

// only nil and false are falsely
IO.write(0 || true) // expect: 0
IO.write(nil || true) // expect: true
IO.write(("" || true) == "") // expect: true
IO.write(false || true) // expect: true
