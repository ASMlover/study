
io.write(Numeric == Numeric) // expect: true
io.write(Numeric == Bool) // expect: false

// not equal to other types
io.write(Numeric == 123) // expect: false
io.write(Numeric == true) // expect: false

io.write(Numeric != Numeric) // expect: false
io.write(Numeric != Bool) // expect: true

io.write(Numeric != 123) // expect: true
io.write(Numeric != true) // expect: true
