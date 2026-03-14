// Test operator overloading

class Vec2 {
  init(x, y) {
    this.x = x;
    this.y = y;
  }

  __add(o) {
    return Vec2(this.x + o.x, this.y + o.y);
  }

  __sub(o) {
    return Vec2(this.x - o.x, this.y - o.y);
  }

  __mul(o) {
    return Vec2(this.x * o.x, this.y * o.y);
  }

  __div(o) {
    return Vec2(this.x / o.x, this.y / o.y);
  }

  __mod(o) {
    return Vec2(this.x % o.x, this.y % o.y);
  }

  __eq(o) {
    return this.x == o.x and this.y == o.y;
  }

  __lt(o) {
    return this.x * this.x + this.y * this.y < o.x * o.x + o.y * o.y;
  }

  __gt(o) {
    return this.x * this.x + this.y * this.y > o.x * o.x + o.y * o.y;
  }

  __str() {
    return "(" + str(this.x) + ", " + str(this.y) + ")";
  }
}

var a = Vec2(1, 2);
var b = Vec2(3, 4);

// Test __add
var c = a + b;
print c.x; // expect: 4
print c.y; // expect: 6

// Test __sub
var d = b - a;
print d.x; // expect: 2
print d.y; // expect: 2

// Test __mul
var e = a * b;
print e.x; // expect: 3
print e.y; // expect: 8

// Test __div
var f = Vec2(10, 20) / Vec2(2, 5);
print f.x; // expect: 5
print f.y; // expect: 4

// Test __mod
var g = Vec2(10, 7) % Vec2(3, 4);
print g.x; // expect: 1
print g.y; // expect: 3

// Test __eq
print a == Vec2(1, 2); // expect: true
print a == b;          // expect: false

// Test __lt / __gt
print Vec2(1, 0) < Vec2(0, 2); // expect: true
print Vec2(0, 2) > Vec2(1, 0); // expect: true

// Test __str via string interpolation
print "${a}"; // expect: (1, 2)
print "${b}"; // expect: (3, 4)

// Test chaining
var h = a + b + Vec2(10, 10);
print h.x; // expect: 14
print h.y; // expect: 16

// Number fast paths still work
print 1 + 2;   // expect: 3
print 10 - 3;  // expect: 7
print 2 * 3;   // expect: 6
print 10 / 2;  // expect: 5
print 1 == 1;  // expect: true
print 1 < 2;   // expect: true
print 2 > 1;   // expect: true

// String fast path still works
print "hello" + " world"; // expect: hello world
