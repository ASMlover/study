// Object-Oriented Programming with class hierarchy

class Shape {
  init(name) {
    this.name = name
  }

  area() {
    return 0
  }

  describe() {
    print this.name + " has area " + this.areaString()
  }

  areaString() {
    return "unknown"
  }
}

class Circle : Shape {
  init(radius) {
    super.init("Circle")
    this.radius = radius
  }

  area() {
    return 3.14159 * this.radius * this.radius
  }

  areaString() {
    // Compute and print area manually since we can't convert number to string
    print "Circle with radius " + this.radius
    return "see above"
  }
}

class Rectangle : Shape {
  init(width, height) {
    super.init("Rectangle")
    this.width = width
    this.height = height
  }

  area() {
    return this.width * this.height
  }
}

// Create shapes
var c = Circle(5)
var r = Rectangle(4, 6)

// Polymorphism
print c.area()    // 78.53975
print r.area()    // 24

// Inheritance chain
class Square : Rectangle {
  init(side) {
    super.init(side, side)
    this.name = "Square"
  }
}

var s = Square(7)
print s.area()    // 49
print s.name      // Square
print s.width     // 7

// Method on base class
class Animal {
  init(name, sound) {
    this.name = name
    this.sound = sound
  }

  speak() {
    print this.name + " says " + this.sound + "!"
  }
}

class Dog : Animal {
  init(name) {
    super.init(name, "Woof")
  }

  fetch(item) {
    print this.name + " fetches the " + item + "!"
  }
}

class Cat : Animal {
  init(name) {
    super.init(name, "Meow")
  }
}

var dog = Dog("Rex")
var cat = Cat("Whiskers")

dog.speak()       // Rex says Woof!
cat.speak()       // Whiskers says Meow!
dog.fetch("ball") // Rex fetches the ball!
