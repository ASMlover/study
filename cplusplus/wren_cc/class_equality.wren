
IO.write(Numeric == Numeric) // expect: true
IO.write(Numeric == Bool) // expect: false

// not equal to other types
IO.write(Numeric == 123) // expect: false
IO.write(Numeric == true) // expect: false

IO.write(Numeric != Numeric) // expect: false
IO.write(Numeric != Bool) // expect: true

IO.write(Numeric != 123) // expect: true
IO.write(Numeric != true) // expect: true
