
IO.print(Function.new {}.arity) // expect: 0
IO.print(Function.new {|a| a }.arity) // expect: 1
IO.print(Function.new {|a, b| a }.arity) // expect: 2
IO.print(Function.new {|a, b, c| a }.arity) // expect: 3
IO.print(Function.new {|a, b, c, d| a }.arity) // expect: 4
