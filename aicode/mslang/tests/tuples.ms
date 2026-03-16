// Tuple type tests

// --- Basic tuple creation ---
var empty = ();
print empty;           // expect: ()

var single = (42,);
print single;          // expect: (42,)

var pair = (1, 2);
print pair;            // expect: (1, 2)

var triple = (1, "hello", true);
print triple;          // expect: (1, "hello", true)

// --- Indexing ---
print pair[0];         // expect: 1
print pair[1];         // expect: 2
print triple[1];       // expect: hello
print triple[2];       // expect: true

// --- Type checking ---
print type(pair);      // expect: tuple
print type(empty);     // expect: tuple

// --- Tuple length ---
print empty.len();     // expect: 0
print single.len();    // expect: 1
print pair.len();      // expect: 2
print triple.len();    // expect: 3

// --- Destructuring ---
var (x, y) = (10, 20);
print x;               // expect: 10
print y;               // expect: 20

var (a, b, c) = (100, 200, 300);
print a;               // expect: 100
print b;               // expect: 200
print c;               // expect: 300

// --- Destructuring from function return ---
fun get_point() {
  return (3, 4);
}
var (px, py) = get_point();
print px;              // expect: 3
print py;              // expect: 4

// --- Nested tuples ---
var nested = ((1, 2), (3, 4));
print nested[0];       // expect: (1, 2)
print nested[1];       // expect: (3, 4)
print nested[0][0];    // expect: 1
print nested[1][1];    // expect: 4

// --- Iteration over tuple ---
var sum = 0;
for (var v in (10, 20, 30)) {
  sum = sum + v;
}
print sum;             // expect: 60

// --- Tuple as map key ---
var m = {};
m[(1, 2)] = "point";
print m[(1, 2)];       // expect: point

// --- Destructuring in local scope ---
{
  var (p, q) = (7, 8);
  print p;             // expect: 7
  print q;             // expect: 8
}

// --- Tuple structural equality ---
var t1 = (1, 2);
var t2 = (1, 2);
print t1 == t2;        // expect: true
print t1 == (1, 3);    // expect: false
print () == ();         // expect: true
