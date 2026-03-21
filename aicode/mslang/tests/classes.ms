// Class declaration and instantiation
print "--- class instantiation ---" // expect: --- class instantiation ---
class Foo {}
var foo = Foo()
print foo                       // expect: Foo instance

// Fields: get and set
print "--- field get and set ---" // expect: --- field get and set ---
foo.x = 42
print foo.x                     // expect: 42
foo.x = 100
print foo.x                     // expect: 100

// Multiple fields
print "--- multiple fields ---" // expect: --- multiple fields ---
foo.y = "hello"
print foo.y                     // expect: hello

// Methods
print "--- methods ---" // expect: --- methods ---
class Greeter {
  greet() {
    print "hello!"
  }
}
var g = Greeter()
g.greet()                       // expect: hello!

// this binding
print "--- this binding ---" // expect: --- this binding ---
class Person {
  init(name) {
    this.name = name
  }
  sayName() {
    print this.name
  }
}
var p = Person("Alice")
p.sayName()                     // expect: Alice
print p.name                    // expect: Alice

// init() returns instance
print "--- init returns instance ---" // expect: --- init returns instance ---
var p2 = Person("Bob")
print p2                        // expect: Person instance

// Methods with arguments
print "--- methods with arguments ---" // expect: --- methods with arguments ---
class Math {
  init(base) {
    this.base = base
  }
  add(x) {
    return this.base + x
  }
}
var m = Math(10)
print m.add(5)                  // expect: 15
print m.add(20)                 // expect: 30

// Inheritance with :
print "--- inheritance ---" // expect: --- inheritance ---
class Animal {
  init(name) {
    this.name = name
  }
  speak() {
    print this.name + " makes a sound."
  }
}

class Dog : Animal {
  speak() {
    print this.name + " barks!"
  }
}

var d = Dog("Rex")
d.speak()                       // expect: Rex barks!
print d.name                    // expect: Rex

// Inherited method
print "--- inherited method ---" // expect: --- inherited method ---
class Cat : Animal {}
var c = Cat("Whiskers")
c.speak()                       // expect: Whiskers makes a sound.

// super calls
print "--- super calls ---" // expect: --- super calls ---
class Puppy : Dog {
  speak() {
    super.speak()
    print "...with a tiny voice."
  }
}
var pup = Puppy("Buddy")
pup.speak()                     // expect: Buddy barks!
                                 // expect: ...with a tiny voice.

// Method override
print "--- method override ---" // expect: --- method override ---
class Base {
  method() {
    return "base"
  }
}
class Derived : Base {
  method() {
    return "derived"
  }
}
print Derived().method()        // expect: derived
print Base().method()           // expect: base

// Bound methods
print "--- bound methods ---" // expect: --- bound methods ---
class Counter {
  init() {
    this.count = 0
  }
  increment() {
    this.count = this.count + 1
    return this.count
  }
}
var cnt = Counter()
var inc = cnt.increment
print inc()                     // expect: 1
print inc()                     // expect: 2
print cnt.count                 // expect: 2

// Class name printing
print "--- class name printing ---" // expect: --- class name printing ---
print Foo                       // expect: Foo
print Animal                    // expect: Animal
