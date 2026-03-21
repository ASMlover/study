// Test abstract methods

// 1. Abstract method on base class raises error when called
class Animal {
  abstract speak() {}
  describe() {
    return "I am an animal"
  }
}

var a = Animal()
print a.describe() // expect: I am an animal

// 2. Subclass overrides abstract method successfully
class Dog : Animal {
  speak() {
    return "Woof!"
  }
}

var d = Dog()
print d.speak()    // expect: Woof!
print d.describe() // expect: I am an animal

// 3. Another subclass overriding abstract method
class Cat : Animal {
  speak() {
    return "Meow!"
  }
}

var c = Cat()
print c.speak() // expect: Meow!

// 4. Calling abstract method on base class should error
// (tested separately in abstract_methods_error.ms)
