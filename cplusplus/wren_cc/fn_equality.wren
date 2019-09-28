
IO.print(Function.new { 123 } == Function.new { 123 }) // expect: false
IO.print(Function.new { 123 } != Function.new { 123 }) // expect: true

IO.print(Function.new { 123 } == 1) // expect: false
IO.print(Function.new { 123 } == false) // expect: false
IO.print(Function.new { 123 } == "fn 123") // expect: false
IO.print(Function.new { 123 } != 1) // expect: true
IO.print(Function.new { 123 } != false) // expect: true
IO.print(Function.new { 123 } != "fn 123") // expect: true

var f = Function.new { 123 }
IO.print(f == f) // expect: true
IO.print(f != f) // expect: false
