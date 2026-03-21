// Test native functions: type, strlen, str, num, input

// --- type() ---
print "--- type() ---" // expect: --- type() ---
print type(nil)           // expect: nil
print type(true)          // expect: bool
print type(false)         // expect: bool
print type(123)           // expect: int
print type(3.14)          // expect: float
print type("hello")       // expect: string
print type(clock)         // expect: function
print type(type)          // expect: function

// --- strlen() ---
print "--- strlen() ---" // expect: --- strlen() ---
print strlen("")          // expect: 0
print strlen("hello")     // expect: 5
print strlen("maple lang") // expect: 10

// --- str() ---
print "--- str() ---" // expect: --- str() ---
print str(nil)            // expect: nil
print str(true)           // expect: true
print str(false)          // expect: false
print str(123)            // expect: 123
print str(3.14)           // expect: 3.14

// --- num() ---
print "--- num() ---" // expect: --- num() ---
print num("0")            // expect: 0
print num("42")           // expect: 42
print num("3.14")         // expect: 3.14
print num("-7")           // expect: -7

// --- type() with class and instance ---
print "--- type() with class/instance ---" // expect: --- type() with class/instance ---
class Foo {}
print type(Foo)           // expect: class
var f = Foo()
print type(f)             // expect: instance
