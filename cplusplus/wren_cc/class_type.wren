
class Foo {}

// a class is a class
IO.print(Foo is Class) // expect: true

// it's metactype is also a class
IO.print(Foo.type is Class) // expect: true

// the metatype's metatype is class
IO.print(Foo.type.type == Class) // expect: true
