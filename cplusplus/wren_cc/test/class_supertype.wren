
class Foo {}

class Bar is Foo {}

class Baz is Bar {}

IO.print(Foo.supertype == Object) // expect: true

IO.print(Bar.supertype == Foo) // expect: true
IO.print(Baz.supertype == Bar) // expect: true

IO.print(Object.supertype) // expect: nil
