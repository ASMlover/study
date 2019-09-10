
IO.print(Numeric.fromString("123") == 123) // expect: true
IO.print(Numeric.fromString("-123") == -123) // expect: true
IO.print(Numeric.fromString("-0") == -0) // expect: true
IO.print(Numeric.fromString("12.34") == 12.34) // expect: true
IO.print(Numeric.fromString("-0.0001") == -0.0001) // expect: true

IO.print(Numeric.fromString("test") == nil) // expect: true
