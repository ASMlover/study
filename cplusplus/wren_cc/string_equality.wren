
io.write("" == "") // expect: true
io.write("abc" == "abc") // expect: true
io.write("abc" == "a") // expect: false
io.write("a" == "abc") // expect: false
io.write("" == "abc") // expect: false

// not equal to other types
io.write("1" == 1) // expect: false
io.write("0" == 0) // expect: false
io.write("true" == true) // expect: false
io.write("false" == false) // expect: false

io.write("" != "") // expect: false
io.write("abc" != "abc") // expect: false
io.write("abc" != "a") // expect: true
io.write("a" != "abc") // expect: true
io.write("" != "abc") // expect: true

// not equal to other types
io.write("1" != 1) // expect: true
io.write("0" != 0) // expect: true
io.write("true" != true) // expect: true
io.write("false" != false) // expect: true
