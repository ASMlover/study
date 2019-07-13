
IO.write("" == "") // expect: true
IO.write("abc" == "abc") // expect: true
IO.write("abc" == "a") // expect: false
IO.write("a" == "abc") // expect: false
IO.write("" == "abc") // expect: false

// not equal to other types
IO.write("1" == 1) // expect: false
IO.write("0" == 0) // expect: false
IO.write("true" == true) // expect: false
IO.write("false" == false) // expect: false

IO.write("" != "") // expect: false
IO.write("abc" != "abc") // expect: false
IO.write("abc" != "a") // expect: true
IO.write("a" != "abc") // expect: true
IO.write("" != "abc") // expect: true

// not equal to other types
IO.write("1" != 1) // expect: true
IO.write("0" != 0) // expect: true
IO.write("true" != true) // expect: true
IO.write("false" != false) // expect: true
