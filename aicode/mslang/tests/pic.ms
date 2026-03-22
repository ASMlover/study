class A { get_x() { return 1; } }
class B { get_x() { return 2; } }
var a1 = A()
var b1 = B()
var a2 = A()
var b2 = B()
var objs = [a1, b1, a2, b2]
var sum = 0
for (var o in objs) { sum = sum + o.get_x(); }
print sum   // expect: 6
