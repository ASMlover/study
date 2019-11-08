
IO.print("something".replace("some", "no")) // expect: nothing
IO.print("something".replace("thing", "one")) // expect: someone
IO.print("something".replace("ometh", "umm")) // expect: summing
IO.print("something".replace("math", "ton")) // expect: something

IO.print("somethingsomething".replace("some", "no")) // expect: nothingnothing
IO.print("abc abc abc".replace(" ", "")) // expect: abcabcabc
IO.print("abcabcabc".replace("abc", "")) // expect:
