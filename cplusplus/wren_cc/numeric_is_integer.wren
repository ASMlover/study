
IO.print(123.isInteger) // expect: true
IO.print(123.0.isInteger) // expect: true
IO.print(0.isInteger) // expect: true
IO.print(1.00001.isInteger) // expect: false
IO.print((-2.3).isInteger) // expect: false

// NaN is not integer
IO.print((0/0).isInteger) // expect: false

// infinity is not integer
IO.print((1/0).isInteger) // expect: false
