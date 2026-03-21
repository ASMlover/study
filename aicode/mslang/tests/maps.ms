// Map literal and print
print "--- map literal ---" // expect: --- map literal ---
var m = {"a": 1, "b": 2, "c": 3}
print m.len()       // expect: 3

// Empty map
print "--- empty map ---" // expect: --- empty map ---
var empty = {}
print empty.len()   // expect: 0

// Index get
print "--- index get ---" // expect: --- index get ---
print m["a"]        // expect: 1
print m["b"]        // expect: 2
print m["c"]        // expect: 3

// Index set (existing key)
print "--- index set (existing key) ---" // expect: --- index set (existing key) ---
m["b"] = 42
print m["b"]        // expect: 42

// Index set (new key)
print "--- index set (new key) ---" // expect: --- index set (new key) ---
m["d"] = 4
print m["d"]        // expect: 4
print m.len()       // expect: 4

// has() method
print "--- has() method ---" // expect: --- has() method ---
print m.has("a")    // expect: true
print m.has("z")    // expect: false

// keys() and values()
print "--- keys() and values() ---" // expect: --- keys() and values() ---
var small = {"x": 10}
var ks = small.keys()
print ks.len()      // expect: 1
print ks[0]         // expect: x

var vs = small.values()
print vs.len()      // expect: 1
print vs[0]         // expect: 10

// remove() method
print "--- remove() method ---" // expect: --- remove() method ---
small.remove("x")
print small.len()   // expect: 0

// Numeric keys
print "--- numeric keys ---" // expect: --- numeric keys ---
var nums = {1: "one", 2: "two"}
print nums[1]       // expect: one
print nums[2]       // expect: two

// Type check
print "--- map type check ---" // expect: --- map type check ---
print type(m)       // expect: map
print type({})      // expect: map

// Map in expressions
print "--- map in expressions ---" // expect: --- map in expressions ---
var prices = {"apple": 3, "banana": 5}
print prices["apple"] + prices["banana"]  // expect: 8

// Mixed key types
print "--- mixed key types ---" // expect: --- mixed key types ---
var mixed = {"name": "Alice", 1: true}
print mixed["name"] // expect: Alice
print mixed[1]      // expect: true
print mixed.len()   // expect: 2
