
// return the first true argument
IO.print(1 || true) // expect: 1
IO.print(false || 1) // expect: 1
IO.print(false || false || true) // expect: true

// return the last argument if all are false
IO.print(false || false) // expect: false
IO.print(false || nil || false) // expect: false

// short-circuit at the first true argument
IO.print(false) || // expect: false
  IO.print(true) || // expect: true
  IO.print(true)

// swallow a trailing newline
IO.print(true ||
  true) // expect: true
