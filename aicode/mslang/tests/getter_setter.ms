// Test: Getter / Setter Properties

class Circle {
  init(r) {
    this._r = r;
  }

  get area() {
    return 3.14 * this._r * this._r;
  }

  get radius() {
    return this._r;
  }

  set radius(v) {
    this._r = v;
  }
}

var c = Circle(5);
print c.area;       // expect: 78.5
print c.radius;     // expect: 5

c.radius = 10;
print c.radius;     // expect: 10
print c.area;       // expect: 314

// Test: getter with inheritance
class Sphere : Circle {
  get volume() {
    return 4.0 / 3.0 * 3.14 * this._r * this._r * this._r;
  }
}

var s = Sphere(3);
print s.area;       // expect: 28.26
print s.volume;     // expect: 113.04

s.radius = 2;
print s.radius;     // expect: 2
print s.area;       // expect: 12.56

// Test: field shadows getter
class Box {
  init() {
    this.size = 42;
  }

  get size() {
    return 99;
  }
}

var b = Box();
print b.size;       // expect: 42
