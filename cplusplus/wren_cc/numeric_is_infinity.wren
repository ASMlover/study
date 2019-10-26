
IO.print(123.isInfinity) // expect: false
IO.print((1/0).isInfinity) // expect: true
IO.print((-10/0).isInfinity) // expect: true
