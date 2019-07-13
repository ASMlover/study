
IO.write(1 < 2) // expect: true
IO.write(2 < 2) // expect: false
IO.write(2 < 1) // expect: false

IO.write(1 <= 2) // expect: true
IO.write(2 <= 2) // expect: true
IO.write(2 <= 1) // expect: false

IO.write(1 > 2) // expect: false
IO.write(2 > 2) // expect: false
IO.write(2 > 1) // expect: true

IO.write(1 >= 2) // expect: false
IO.write(2 >= 2) // expect: true
IO.write(2 >= 1) // expect: true

IO.write(0 < -0) // expect: false
IO.write(-0 < 0) // expect: false
IO.write(0 > -0) // expect: false
IO.write(-0 > 0) // expect: false
IO.write(0 <= -0) // expect: true
IO.write(-0 <= 0) // expect: true
IO.write(0 >= -0) // expect: true
IO.write(-0 >= 0) // expect: true
