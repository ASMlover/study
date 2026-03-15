// Superinstruction fusion: GET_LOCAL+GET_LOCAL+binary_op → BINARY_LOCAL_LOCAL

// Basic local+local arithmetic
{
  var a = 10;
  var b = 3;
  print a + b;   // expect: 13
  print a - b;   // expect: 7
  print a * b;   // expect: 30
  print a / b;   // expect: 3.33333
  print a % b;   // expect: 1
}

// Nested expressions with locals
{
  var x = 5;
  var y = 2;
  var z = x + y;
  print z;        // expect: 7
  print z * x;    // expect: 35
  print z - y;    // expect: 5
}

// In functions
fun add(a, b) {
  return a + b;
}
print add(100, 200);  // expect: 300

fun arithmetic(x, y) {
  var sum = x + y;
  var diff = x - y;
  var prod = x * y;
  var quot = x / y;
  var mod = x % y;
  print sum;   // expect: 15
  print diff;  // expect: 5
  print prod;  // expect: 50
  print quot;  // expect: 2
  print mod;   // expect: 0
}
arithmetic(10, 5);

// In loops (hot path optimization)
{
  var total = 0;
  var step = 3;
  for (var i = 0; i < 5; i += 1) {
    total = total + step;
  }
  print total;  // expect: 15
}

// With closures (upvalues should NOT fuse, only locals)
{
  var outer = 10;
  fun inner(x) {
    return x + outer;  // outer is upvalue, x is local — no fusion
  }
  print inner(5);  // expect: 15
}

// String concatenation with local+local
{
  var s1 = "hello";
  var s2 = " world";
  print s1 + s2;  // expect: hello world
}

// Chained operations
{
  var a = 2;
  var b = 3;
  var c = 4;
  print a + b + c;   // expect: 9
  print a * b + c;   // expect: 10
  print a + b * c;   // expect: 14
}

// Interaction with constant folding (constants should still fold)
{
  print 1 + 2;       // expect: 3
  var x = 10;
  print x + 5;       // expect: 15
}
