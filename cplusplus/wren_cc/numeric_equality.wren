
io.write(123 == 123) // expect: true
io.write(123 == 234) // expect: false
io.write(-123 == 123) // expect: false
io.write(0 == -0) // expect: true

// not equal to other types
io.write(123 == "123") // expect: false
io.write(1 == true) // expect: false
io.write(0 == false) // expect: false

io.write(123 != 123) // expect: false
io.write(123 != 124) // expect: true
io.write(-123 != 123) // expect: true
io.write(0 != -0) // expect: false

// not equal to other types
io.write(123 != "123") // expect: true
io.write(1 != true) // expect: true
io.write(0 != false) // expect: true
