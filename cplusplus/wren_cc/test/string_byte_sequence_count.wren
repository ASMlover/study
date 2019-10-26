
// simple
IO.print("".bytes.count) // expect: 0
IO.print("123".bytes.count) // expect: 3

// null bytes
IO.print("\0\0\0".bytes.count) // expect: 3
