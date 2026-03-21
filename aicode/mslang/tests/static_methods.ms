// Static methods tests

// Basic static method
class Math {
  static max(a, b) {
    if (a > b) return a
    return b
  }

  static min(a, b) {
    if (a < b) return a
    return b
  }
}

print Math.max(3, 5)    // expect: 5
print Math.min(3, 5)    // expect: 3
print Math.max(10, 2)   // expect: 10

// Static method with no args
class Counter {
  static zero() { return 0; }
}
print Counter.zero()    // expect: 0

// Static and instance methods on same class
class Foo {
  init(x) {
    this.x = x
  }

  get_x() {
    return this.x
  }

  static create(val) {
    return Foo(val)
  }
}

var f = Foo.create(42)
print f.get_x()         // expect: 42

// Static methods are inherited
class Base {
  static greet() { return "hello"; }
}

class Child : Base {}

print Child.greet()     // expect: hello

// Override static method in subclass
class Parent {
  static name() { return "Parent"; }
}

class Sub : Parent {
  static name() { return "Sub"; }
}

print Parent.name()     // expect: Parent
print Sub.name()        // expect: Sub

// Static method as first-class value
class Utils {
  static double(x) { return x * 2; }
}

var fn = Utils.double
print fn(7)             // expect: 14

// Multiple static methods
class StringUtils {
  static repeat(s, n) {
    var result = ""
    for (var i = 0; i < n; i = i + 1) {
      result = result + s
    }
    return result
  }

  static is_empty(s) {
    return s == ""
  }
}

print StringUtils.repeat("ab", 3)  // expect: ababab
print StringUtils.is_empty("")     // expect: true
print StringUtils.is_empty("hi")   // expect: false
