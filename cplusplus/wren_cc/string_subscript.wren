
IO.write("abcd"[0]) // expect: a
IO.write("abcd"[1]) // expect: b
IO.write("abcd"[2]) // expect: c
IO.write("abcd"[3]) // expect: d

IO.write("abcd"[-4]) // expect: a
IO.write("abcd"[-3]) // expect: b
IO.write("abcd"[-2]) // expect: c
IO.write("abcd"[-1]) // expect: d
