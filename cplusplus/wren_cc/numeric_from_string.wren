
IO.print(Numeric.fromString("123") == 123) // expect: true
IO.print(Numeric.fromString("-123") == -123) // expect: true
IO.print(Numeric.fromString("-0") == -0) // expect: true
IO.print(Numeric.fromString("12.34") == 12.34) // expect: true
IO.print(Numeric.fromString("-0.0001") == -0.0001) // expect: true
IO.print(Numeric.fromString(" 12 ") == 12) // expect: true
IO.print(Numeric.fromString("1.2prefix") == 1.2) // expect: true

IO.print(Numeric.fromString("test") == nil) // expect: true
IO.print(Numeric.fromString("") == nil) // expect: true
IO.print(Numeric.fromString("prefix1.2") == nil) // expect: true
