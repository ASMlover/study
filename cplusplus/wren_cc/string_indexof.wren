
IO.print("abcd".indexOf("")) // expect: 0
IO.print("abcd".indexOf("cd")) // expect: 2
IO.print("abcd".indexOf("a")) // expect: 0
IO.print("abcd".indexOf("abcd")) // expect: 0
IO.print("abcd".indexOf("abcde")) // expect: -1
IO.print("abcd".indexOf("ab")) // expect: 0

IO.print("abcdefabcdefg".indexOf("defg")) // expect: 9
IO.print("abcdabcdabcd".indexOf("dab")) // expect: 3
IO.print("abcdabcdabcdabcd".indexOf("dabcdabc")) // expect: 3
IO.print("abcdefg".indexOf("abcdef!")) // expect: -1
