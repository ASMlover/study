
var a = [1, 2, 3]
IO.print(a.all{|x| x > 1 }) // expect: false
IO.print(a.all{|x| x > 0 }) // expect: true
IO.print([].all{|x| false }) // expect: true

// returns first false value
IO.print(a.all{|x| x < 2 ? nil : false }) // expect: nil

// returns last true value
IO.print(a.all{|x| x }) // expect: 3
