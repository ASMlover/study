// If-else
if (true) {
  print "yes";           // expect: yes
}

if (false) {
  print "no";
} else {
  print "else";          // expect: else
}

// Truthiness
if (0) {
  print "zero is truthy";  // expect: zero is truthy
}

if (nil) {
  print "nil is truthy";
} else {
  print "nil is falsey";   // expect: nil is falsey
}

if ("") {
  print "empty string is truthy";  // expect: empty string is truthy
}

// While loop
var i = 0;
while (i < 5) {
  i = i + 1;
}
print i;                 // expect: 5

// For loop
var sum = 0;
for (var j = 1; j <= 10; j = j + 1) {
  sum = sum + j;
}
print sum;               // expect: 55

// Logical operators
print true and true;     // expect: true
print true and false;    // expect: false
print false and true;    // expect: false
print false or true;     // expect: true
print false or false;    // expect: false

// Short-circuit
var x = "ok";
false and (x = "bad");
print x;                 // expect: ok

true or (x = "bad");
print x;                 // expect: ok
