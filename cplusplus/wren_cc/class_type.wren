
class Foo {}

// a class is a class
IO.write(Foo is Class) // expect: true

// it's metactype is also a class
IO.write(Foo.type is Class) // expect: true

// the metatype's metatype is class
IO.write(Foo.type.type == Class) // expect: true
