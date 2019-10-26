
var a = [1, 2, 3]
IO.print(a.any{|x| x > 3 }) // expect: false
IO.print(a.any{|x| x > 1 }) // expect: true
IO.print([].any{|x| true }) // expect: false

// returns first true value
IO.print(a.any{|x| x }) // expect: 1

// returns last false value
IO.print(a.any{|x| x < 2 ? nil : false }) // expect: false
