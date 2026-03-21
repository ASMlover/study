// If-else
print "--- if-else ---"; // expect: --- if-else ---
if (true) {
  print "yes";           // expect: yes
}

if (false) {
  print "no";
} else {
  print "else";          // expect: else
}

// Truthiness
print "--- truthiness ---"; // expect: --- truthiness ---
if (0) {
  print "zero is truthy";
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
print "--- while loop ---"; // expect: --- while loop ---
var i = 0;
while (i < 5) {
  i = i + 1;
}
print i;                 // expect: 5

// For loop
print "--- for loop ---"; // expect: --- for loop ---
var sum = 0;
for (var j = 1; j <= 10; j = j + 1) {
  sum = sum + j;
}
print sum;               // expect: 55

// Logical operators
print "--- logical operators ---"; // expect: --- logical operators ---
print true and true;     // expect: true
print true and false;    // expect: false
print false and true;    // expect: false
print false or true;     // expect: true
print false or false;    // expect: false

// Short-circuit
print "--- short-circuit evaluation ---"; // expect: --- short-circuit evaluation ---
var x = "ok";
false and (x = "bad");
print x;                 // expect: ok

true or (x = "bad");
print x;                 // expect: ok

// else if chain
print "--- else-if chain ---"; // expect: --- else-if chain ---
var n = 5;
if (n < 0) {
  print "negative";
} else if (n == 0) {
  print "zero";
} else if (n < 10) {
  print "small";         // expect: small
} else {
  print "big";
}

// nested else if
print "--- nested else-if ---"; // expect: --- nested else-if ---
var grade = 85;
if (grade >= 90) {
  print "A";
} else if (grade >= 80) {
  print "B";             // expect: B
} else if (grade >= 70) {
  print "C";
} else if (grade >= 60) {
  print "D";
} else {
  print "F";
}
