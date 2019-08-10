
IO.print(new Function { 123 }) == new Function { 123 }) // expect: false
IO.print(new Function { 123 }) != new Function { 123 }) // expect: true

IO.print(new Function { 123 }) == 1) // expect: false
IO.print(new Function { 123 }) == false) // expect: false
IO.print(new Function { 123 }) == "fn 123") // expect: false
IO.print(new Function { 123 }) != 1) // expect: true
IO.print(new Function { 123 }) != false) // expect: true
IO.print(new Function { 123 }) != "fn 123") // expect: true

var f = new Function { 123 }
IO.print(f == f) // expect: true
IO.print(f != f) // expect: false
