// String literals
print "--- string literals ---" // expect: --- string literals ---
print "hello"           // expect: hello
print "world"           // expect: world

// String concatenation
print "--- string concatenation ---" // expect: --- string concatenation ---
print "hello" + " " + "world"  // expect: hello world

// String variables
print "--- string variables ---" // expect: --- string variables ---
var greeting = "Hi"
var name = "Maple"
print greeting + ", " + name + "!"  // expect: Hi, Maple!

// String equality
print "--- string equality ---" // expect: --- string equality ---
print "abc" == "abc"    // expect: true
print "abc" == "def"    // expect: false
print "abc" != "def"    // expect: true

// Empty string
print "--- empty string ---" // expect: --- empty string ---
print ""                // expect:

// Nil and boolean printing
print "--- nil and boolean printing ---" // expect: --- nil and boolean printing ---
print nil               // expect: nil
print true              // expect: true
print false             // expect: false
