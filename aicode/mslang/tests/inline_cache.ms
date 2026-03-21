// Test: Inline caching for property access

// === Basic field access (monomorphic IC) ===
class Point {
  init(x, y) {
    this.x = x
    this.y = y
  }
}

var p = Point(3, 4)
print p.x           // expect: 3
print p.y           // expect: 4
p.x = 10
print p.x           // expect: 10

// === Method invocation IC ===
class Counter {
  init() {
    this.count = 0
  }
  increment() {
    this.count = this.count + 1
  }
  value() {
    return this.count
  }
}

var c = Counter()
c.increment()
c.increment()
c.increment()
print c.value()     // expect: 3

// === IC with multiple instances of same class ===
var p1 = Point(1, 2)
var p2 = Point(5, 6)
print p1.x + p2.x  // expect: 6
print p1.y + p2.y  // expect: 8

// === Getter IC ===
class Circle {
  init(r) {
    this._r = r
  }
  get area() {
    return 3.14 * this._r * this._r
  }
}

var circle = Circle(10)
print circle.area   // expect: 314

// === Setter IC ===
class Box {
  init() {
    this._w = 0
  }
  set width(v) {
    this._w = v
  }
  get width() {
    return this._w
  }
}

var b = Box()
b.width = 42
print b.width       // expect: 42

// === Polymorphic site (class mismatch invalidates IC) ===
class Dog {
  init(name) { this.name = name; }
  speak() { return "woof"; }
}
class Cat {
  init(name) { this.name = name; }
  speak() { return "meow"; }
}

fun get_sound(animal) {
  return animal.speak()
}

print get_sound(Dog("Rex"))  // expect: woof
print get_sound(Cat("Mia"))  // expect: meow
print get_sound(Dog("Buddy")) // expect: woof

// === Inheritance with IC ===
class Animal {
  init(name) { this.name = name; }
  describe() { return this.name; }
}
class Parrot : Animal {
  init(name, color) {
    super.init(name)
    this.color = color
  }
}

var parrot = Parrot("Polly", "green")
print parrot.describe()  // expect: Polly
print parrot.color       // expect: green

// === Invoke in loop (IC hot path) ===
class Adder {
  init() { this.sum = 0; }
  add(n) { this.sum = this.sum + n; }
}

var adder = Adder()
for (var i = 1; i <= 100; i = i + 1) {
  adder.add(i)
}
print adder.sum     // expect: 5050

// === Static method access (no IC, should still work) ===
class MathUtil {
  static square(x) { return x * x; }
}
print MathUtil.square(7)  // expect: 49

// === Module property access (no IC, should still work) ===
// (Skipping module test — requires file import)

// === Operator overloading with IC ===
class Vec {
  init(x) { this.x = x; }
  __add(other) { return Vec(this.x + other.x); }
  __str() { return str(this.x); }
}

var v1 = Vec(10)
var v2 = Vec(20)
var v3 = v1 + v2
print "${v3}"       // expect: 30

print "all inline cache tests passed"  // expect: all inline cache tests passed
