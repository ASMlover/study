
IO.print("abcd".indexOf("", 0)) // expect: 0
IO.print("abcd".indexOf("", 1)) // expect: 1
IO.print("abcd".indexOf("", 2)) // expect: 2

IO.print("aaaaa".indexOf("aaaa", 0)) // expect: 0
IO.print("aaaaa".indexOf("aaaa", 1)) // expect: 1
IO.print("aaaaa".indexOf("aaaa", 2)) // expect: -1

IO.print("abcd".indexOf("abcde", -1)) // expect: -1
IO.print("abcd".indexOf("cde", 3)) // expect: -1

IO.print("abcd".indexOf("cd", 0)) // expect: 2
IO.print("abcd".indexOf("cd", 1)) // expect: 2
IO.print("abcd".indexOf("cd", 2)) // expect: 2
IO.print("abcd".indexOf("cd", 3)) // expect: -1

IO.print("abcd".indexOf("cd", -4)) // expect: 2
IO.print("abcd".indexOf("cd", -3)) // expect: 2
IO.print("abcd".indexOf("cd", -2)) // expect: 2
IO.print("abcd".indexOf("cd", -1)) // expect: -1

IO.print("here as well as here".indexOf("here", 1)) // expect: 16
