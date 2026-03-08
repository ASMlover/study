// Map literal and print
var m = {"a": 1, "b": 2, "c": 3};
print m.len();       // expect: 3

// Empty map
var empty = {};
print empty.len();   // expect: 0

// Index get
print m["a"];        // expect: 1
print m["b"];        // expect: 2
print m["c"];        // expect: 3

// Index set (existing key)
m["b"] = 42;
print m["b"];        // expect: 42

// Index set (new key)
m["d"] = 4;
print m["d"];        // expect: 4
print m.len();       // expect: 4

// has() method
print m.has("a");    // expect: true
print m.has("z");    // expect: false

// keys() and values()
var small = {"x": 10};
var ks = small.keys();
print ks.len();      // expect: 1
print ks[0];         // expect: x

var vs = small.values();
print vs.len();      // expect: 1
print vs[0];         // expect: 10

// remove() method
small.remove("x");
print small.len();   // expect: 0

// Numeric keys
var nums = {1: "one", 2: "two"};
print nums[1];       // expect: one
print nums[2];       // expect: two

// Type check
print type(m);       // expect: map
print type({});      // expect: map

// Map in expressions
var prices = {"apple": 3, "banana": 5};
print prices["apple"] + prices["banana"];  // expect: 8

// Mixed key types
var mixed = {"name": "Alice", 1: true};
print mixed["name"]; // expect: Alice
print mixed[1];      // expect: true
print mixed.len();   // expect: 2
