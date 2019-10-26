
var bytes = "some".bytes

IO.print(bytes[0]) // expect: 115
IO.print(bytes[1]) // expect: 111
IO.print(bytes[2]) // expect: 109
IO.print(bytes[3]) // expect: 101

IO.print(bytes[-4]) // expect: 115
IO.print(bytes[-3]) // expect: 111
IO.print(bytes[-2]) // expect: 109
IO.print(bytes[-1]) // expect: 101
