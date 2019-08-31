
IO.print(new Function {}.arity) // expect: 0
IO.print(new Function {|a| a }.arity) // expect: 1
IO.print(new Function {|a, b| a }.arity) // expect: 2
IO.print(new Function {|a, b, c| a }.arity) // expect: 3
IO.print(new Function {|a, b, c, d| a }.arity) // expect: 4
