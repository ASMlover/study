
// false and nil are false
IO.print(false || "ok") // expect: ok
IO.print(nil || "ok") // expect: ok

// everything else is true
IO.print(true || "ok") // expect: true
IO.print(0 || "ok") // expect: 0
IO.print("s" || "ok") // expect: s
