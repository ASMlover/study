
IO.print((fn 123) == (fn 123)) // expect: false
IO.print((fn 123) != (fn 123)) // expect: true

IO.print((fn 123) == 1) // expect: false
IO.print((fn 123) == false) // expect: false
IO.print((fn 123) == "fn 123") // expect: false
IO.print((fn 123) != 1) // expect: true
IO.print((fn 123) != false) // expect: true
IO.print((fn 123) != "fn 123") // expect: true

var f = fn 123
IO.print(f == f) // expect: true
IO.print(f != f) // expect: false
