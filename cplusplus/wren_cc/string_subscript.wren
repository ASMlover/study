
io.write("abcd"[0]) // expect: a
io.write("abcd"[1]) // expect: b
io.write("abcd"[2]) // expect: c
io.write("abcd"[3]) // expect: d

io.write("abcd"[-4]) // expect: a
io.write("abcd"[-3]) // expect: b
io.write("abcd"[-2]) // expect: c
io.write("abcd"[-1]) // expect: d

io.write("abcd"[4]) // expect: nil
io.write("abcd"[-5]) // expect: nil

io.write("abcd"[true]) // expect: nil
io.write("abcd"[1.5]) // expect: nil
