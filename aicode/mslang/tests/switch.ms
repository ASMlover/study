// --- basic switch ---

var x = 2;
switch (x) {
  case 1:
    print "one";
  case 2:
    print "two";    // expect: two
  case 3:
    print "three";
  default:
    print "other";
}

// --- default only ---

var y = 99;
switch (y) {
  case 1:
    print "one";
  default:
    print "default"; // expect: default
}

// --- first case matches ---

var z = 1;
switch (z) {
  case 1:
    print "first";   // expect: first
  case 2:
    print "second";
}

// --- string cases ---

var s = "hello";
switch (s) {
  case "world":
    print "world";
  case "hello":
    print "hello";   // expect: hello
  default:
    print "unknown";
}

// --- nil case ---

var n = nil;
switch (n) {
  case nil:
    print "is nil";  // expect: is nil
  default:
    print "not nil";
}

// --- expression as case value ---

var a = 10;
switch (a) {
  case 5 + 5:
    print "ten";     // expect: ten
  default:
    print "not ten";
}

// --- nested switch ---

var outer = 1;
var inner = 2;
switch (outer) {
  case 1:
    switch (inner) {
      case 1:
        print "1-1";
      case 2:
        print "1-2"; // expect: 1-2
      default:
        print "1-?";
    }
  case 2:
    print "2";
}

// --- switch with break ---

var b = 1;
switch (b) {
  case 1:
    print "matched"; // expect: matched
    break;
  case 2:
    print "two";
}

// --- empty switch ---

switch (42) {
  default:
    print "only default"; // expect: only default
}

// --- no matching case, no default ---

switch (999) {
  case 1:
    print "nope";
  case 2:
    print "nope";
}
// (no output expected)

print "done"; // expect: done
