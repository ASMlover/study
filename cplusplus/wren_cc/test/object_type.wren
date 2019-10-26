
class Foo {}

IO.print(Object is Class) // expect: true
IO.print(Object.type is Class) // expect: true
IO.print(Object.type.type == Class) // expect: true
IO.print(Object.type.name) // expect: Object metaclass
