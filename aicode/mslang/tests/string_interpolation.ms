// Basic string interpolation
print "--- basic interpolation ---" // expect: --- basic interpolation ---
var name = "World"
print "Hello, ${name}!" // expect: Hello, World!

// Expression inside interpolation
print "--- expression interpolation ---" // expect: --- expression interpolation ---
print "1 + 2 = ${1 + 2}" // expect: 1 + 2 = 3

// Multiple interpolations
print "--- multiple interpolations ---" // expect: --- multiple interpolations ---
var a = "foo"
var b = "bar"
print "${a} and ${b}" // expect: foo and bar

// Interpolation with no surrounding text
print "--- bare interpolation ---" // expect: --- bare interpolation ---
print "${42}" // expect: 42

// Adjacent interpolations
print "--- adjacent interpolations ---" // expect: --- adjacent interpolations ---
print "${1}${2}${3}" // expect: 123

// Interpolation with different types
print "--- type coercion ---" // expect: --- type coercion ---
print "nil: ${nil}" // expect: nil: nil
print "bool: ${true}" // expect: bool: true
print "num: ${3.14}" // expect: num: 3.14

// Interpolation with function call
print "--- function call ---" // expect: --- function call ---
fun greet(n) { return "Hi, " + n }
print "says: ${greet("Maple")}" // expect: says: Hi, Maple

// Interpolation with method call
print "--- method call ---" // expect: --- method call ---
var list = [1, 2, 3]
print "len = ${list.len()}" // expect: len = 3

// Escaped interpolation
print "--- escaped interpolation ---" // expect: --- escaped interpolation ---
print "price is \${value}" // expect: price is ${value}

// String with only interpolation
print "--- only interpolation ---" // expect: --- only interpolation ---
var x = "hello"
print "${x}" // expect: hello

// Interpolation with arithmetic
print "--- arithmetic in interpolation ---" // expect: --- arithmetic in interpolation ---
var price = 10
var qty = 3
print "Total: ${price * qty}" // expect: Total: 30

// Nested string inside interpolation expression
print "--- string in interpolation ---" // expect: --- string in interpolation ---
print "result: ${"a" + "b"}" // expect: result: ab

// Empty segments
print "--- empty first segment ---" // expect: --- empty first segment ---
print "${100} percent" // expect: 100 percent

print "--- empty last segment ---" // expect: --- empty last segment ---
print "value: ${99}" // expect: value: 99

// Interpolation with comparison
print "--- comparison in interpolation ---" // expect: --- comparison in interpolation ---
print "is big: ${100 > 50}" // expect: is big: true

// Mixed regular and interpolated strings
print "--- mixed strings ---" // expect: --- mixed strings ---
var msg = "hello ${name}"
var plain = "world"
print msg + " " + plain // expect: hello World world
