
var range = 1..10

IO.print(range.reduce{|x, y| x + y }) // expect: 55
IO.print(range.reduce(100) {|x, y| x < y ? x : y }) // expect: 1
