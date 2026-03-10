// Compound assignment on locals
print "--- compound assignment on locals ---"; // expect: --- compound assignment on locals ---
var x = 10;
x += 5;
print x;           // expect: 15
x -= 3;
print x;           // expect: 12
x *= 2;
print x;           // expect: 24
x /= 4;
print x;           // expect: 6
x %= 4;
print x;           // expect: 2

// Compound assignment on globals
print "--- compound assignment on globals ---"; // expect: --- compound assignment on globals ---
var g = 100;
fun modify() {
  g += 50;
}
modify();
print g;            // expect: 150

// Compound assignment on upvalues
print "--- compound assignment on upvalues ---"; // expect: --- compound assignment on upvalues ---
fun make_counter() {
  var count = 0;
  return fun() {
    count += 1;
    return count;
  };
}
var counter = make_counter();
print counter();    // expect: 1
print counter();    // expect: 2
print counter();    // expect: 3

// String concatenation with +=
print "--- string concatenation with += ---"; // expect: --- string concatenation with += ---
var s = "hello";
s += " world";
print s;            // expect: hello world
