// String methods tests

var s = "Hello, World!";

// len
print s.len();              // expect: 13

// slice
print s.slice(0, 5);       // expect: Hello
print s.slice(7, 12);      // expect: World
print s.slice(-6, -1);     // expect: World

// find
print s.find("World");     // expect: 7
print s.find("xyz");       // expect: -1

// replace
print s.replace("World", "Maple");  // expect: Hello, Maple!

// split
var parts = "a,b,c".split(",");
print parts.len();          // expect: 3
print parts[0];             // expect: a
print parts[1];             // expect: b
print parts[2];             // expect: c

// split with empty delimiter
var chars = "hi".split("");
print chars.len();          // expect: 2
print chars[0];             // expect: h
print chars[1];             // expect: i

// upper / lower
print "hello".upper();     // expect: HELLO
print "HELLO".lower();     // expect: hello

// trim
print "  hi  ".trim();     // expect: hi

// starts_with / ends_with / contains
print s.starts_with("Hello");  // expect: true
print s.starts_with("World");  // expect: false
print s.ends_with("!");        // expect: true
print s.ends_with("World");    // expect: false
print s.contains("World");     // expect: true
print s.contains("xyz");       // expect: false
