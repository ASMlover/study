
IO.print(new Function { 0 } is Function) // expect: true
IO.print(new Function { 0 } is Object) // expect: true
IO.print(new Function { 0 } is String) // expect: false
IO.print(new Function { 0 }.type == Function) // expect: true
