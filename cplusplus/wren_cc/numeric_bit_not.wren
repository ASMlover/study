
IO.print(~0) // expect: 4294967295
IO.print(~1) // expect: 4294967294
IO.print(~23) // expect: 4294967272

IO.print(~4294967295) // expect: 0
IO.print(~4294967296) // expect: 4294967295

// negative numerics
IO.print(~-1) // expect: 0
IO.print(~-123) // expect: 122
IO.print(~-4294967294) // expect: 4294967293
IO.print(~-4294967295) // expect: 4294967294
IO.print(~-4294967296) // expect: 4294967295

// floating point values
IO.print(~1.23) // expect: 4294967294
IO.print(~0.00123) // expect: 4294967295
IO.print(~345.67) // expect: 4294966950
IO.print(~-12.34) // expect: 11
