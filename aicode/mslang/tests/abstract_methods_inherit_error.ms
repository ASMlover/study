// Test subclass inherits abstract method and still errors if not overridden
class Shape {
  abstract area() {}
  abstract perimeter() {}
}

class Circle : Shape {
  init(r) { this.r = r; }
  area() { return 3.14 * this.r * this.r; }
  // perimeter not overridden — should still be abstract
}

var c = Circle(5);
print c.area(); // expect: 78.5
c.perimeter();  // expect runtime error
