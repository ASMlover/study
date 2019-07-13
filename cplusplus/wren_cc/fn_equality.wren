
IO.write((fn 123) == (fn 123)) // expect: false
IO.write((fn 123) != (fn 123)) // expect: true

IO.write((fn 123) == 1) // expect: false
IO.write((fn 123) == false) // expect: false
IO.write((fn 123) == "fn 123") // expect: false
IO.write((fn 123) != 1) // expect: true
IO.write((fn 123) != false) // expect: true
IO.write((fn 123) != "fn 123") // expect: true

var f = fn 123
IO.write(f == f) // expect: true
IO.write(f != f) // expect: false
