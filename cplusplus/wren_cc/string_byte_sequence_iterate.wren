
var bytes = "some".bytes

IO.print(bytes.iterate(nil)) // expect: 0
IO.print("".bytes.iterate(nil)) // expect: false

IO.print(bytes.iterate(0)) // expect: 1
IO.print(bytes.iterate(1)) // expect: 2
IO.print(bytes.iterate(2)) // expect: 3
IO.print(bytes.iterate(3)) // expect: false

IO.print(bytes.iterate(123)) // expect: false
IO.print(bytes.iterate(-1)) // expect: false
