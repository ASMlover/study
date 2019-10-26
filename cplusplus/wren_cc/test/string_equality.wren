
IO.print("" == "") // expect: true
IO.print("abc" == "abc") // expect: true
IO.print("abc" == "a") // expect: false
IO.print("a" == "abc") // expect: false
IO.print("" == "abc") // expect: false

// not equal to other types
IO.print("1" == 1) // expect: false
IO.print("0" == 0) // expect: false
IO.print("true" == true) // expect: false
IO.print("false" == false) // expect: false

IO.print("" != "") // expect: false
IO.print("abc" != "abc") // expect: false
IO.print("abc" != "a") // expect: true
IO.print("a" != "abc") // expect: true
IO.print("" != "abc") // expect: true

// not equal to other types
IO.print("1" != 1) // expect: true
IO.print("0" != 0) // expect: true
IO.print("true" != true) // expect: true
IO.print("false" != false) // expect: true
