
IO.print((2..5).contains(1)) // expect: false
IO.print((2..5).contains(2)) // expect: true
IO.print((2..5).contains(5)) // expect: true
IO.print((2..5).contains(6)) // expect: false

IO.print((5..2).contains(1)) // expect: false
IO.print((5..2).contains(2)) // expect: true
IO.print((5..2).contains(5)) // expect: true
IO.print((5..2).contains(6)) // expect: false

IO.print((2...5).contains(1)) // expect: false
IO.print((2...5).contains(2)) // expect: true
IO.print((2...5).contains(5)) // expect: false
IO.print((2...5).contains(6)) // expect: false

IO.print((5...2).contains(1)) // expect: false
IO.print((5...2).contains(2)) // expect: false
IO.print((5...2).contains(5)) // expect: true
IO.print((5...2).contains(6)) // expect: false
