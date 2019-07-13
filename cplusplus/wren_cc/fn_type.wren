
IO.write((fn 0) is FunctIO.) // expect: true
IO.write((fn 0) is Object) // expect: true
IO.write((fn 0) is String) // expect: false
IO.write((fn 0).type == FunctIO.) // expect: true
