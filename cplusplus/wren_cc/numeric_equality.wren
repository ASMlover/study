
IO.print(123 == 123) // expect: true
IO.print(123 == 234) // expect: false
IO.print(-123 == 123) // expect: false
IO.print(0 == -0) // expect: true

// not equal to other types
IO.print(123 == "123") // expect: false
IO.print(1 == true) // expect: false
IO.print(0 == false) // expect: false

IO.print(123 != 123) // expect: false
IO.print(123 != 124) // expect: true
IO.print(-123 != 123) // expect: true
IO.print(0 != -0) // expect: false

// not equal to other types
IO.print(123 != "123") // expect: true
IO.print(1 != true) // expect: true
IO.print(0 != false) // expect: true
