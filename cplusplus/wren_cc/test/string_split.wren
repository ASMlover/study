
IO.print("something".split("meth")) // expect: [so, ing]
IO.print("something".split("some")) // expect: [, thing]
IO.print("something".split("ing")) // expect: [someth, ]
IO.print("something".split("math")) // expect: [something]

IO.print("somethingsomething".split("meth")) // expect: [so, ingso, ing]
IO.print("abc abc abc".split(" ")) // expect: [abc, abc, abc]
IO.print("abcabcabc".split("abc")) // expect: [, , , ]
