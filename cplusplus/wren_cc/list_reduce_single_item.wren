
IO.print([1].reduce{|x, y| 42 }) // expect: 1
IO.print([].reduce(1) {|x, y| 42 }) // expect: 1
