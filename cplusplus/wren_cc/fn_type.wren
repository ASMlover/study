
IO.print((fn 0) is FunctIO.) // expect: true
IO.print((fn 0) is Object) // expect: true
IO.print((fn 0) is String) // expect: false
IO.print((fn 0).type == FunctIO.) // expect: true
