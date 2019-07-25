
IO.print(1 < 2) // expect: true
IO.print(2 < 2) // expect: false
IO.print(2 < 1) // expect: false

IO.print(1 <= 2) // expect: true
IO.print(2 <= 2) // expect: true
IO.print(2 <= 1) // expect: false

IO.print(1 > 2) // expect: false
IO.print(2 > 2) // expect: false
IO.print(2 > 1) // expect: true

IO.print(1 >= 2) // expect: false
IO.print(2 >= 2) // expect: true
IO.print(2 >= 1) // expect: true

IO.print(0 < -0) // expect: false
IO.print(-0 < 0) // expect: false
IO.print(0 > -0) // expect: false
IO.print(-0 > 0) // expect: false
IO.print(0 <= -0) // expect: true
IO.print(-0 <= 0) // expect: true
IO.print(0 >= -0) // expect: true
IO.print(-0 >= 0) // expect: true
