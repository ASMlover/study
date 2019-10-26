
// false and nil are false
IO.print(false && "bad") // expect: false
IO.print(nil && "bad") // expect: nil

// everything else is true
IO.print(true && "ok") // expect: ok
IO.print(0 && "ok") // expect: ok
IO.print("" && "ok") // expect: ok
