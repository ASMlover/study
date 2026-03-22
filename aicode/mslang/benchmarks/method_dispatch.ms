class Dog {
  speak() { return 1 }
}
class Cat {
  speak() { return 2 }
}
var d = Dog()
var c = Cat()
var sum = 0
var i = 0
while (i < 500000) {
  sum = sum + d.speak()
  sum = sum + c.speak()
  i += 1
}
print sum   // expect: 1500000
