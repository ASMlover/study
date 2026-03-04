// Basic function declaration and call
fun add(a, b) {
  return a + b;
}
print add(1, 2);         // expect: 3

// Function with no return (returns nil)
fun greet(name) {
  print "Hello, " + name + "!";
}
greet("Maple");          // expect: Hello, Maple!
print greet("World");    // expect: Hello, World!
                         // expect: nil

// Recursion: fibonacci
fun fib(n) {
  if (n < 2) return n;
  return fib(n - 1) + fib(n - 2);
}
print fib(0);            // expect: 0
print fib(1);            // expect: 1
print fib(10);           // expect: 55

// Multiple return values through different paths
fun abs(n) {
  if (n < 0) return -n;
  return n;
}
print abs(-5);           // expect: 5
print abs(3);            // expect: 3

// Nested functions
fun outer() {
  fun inner() {
    return "inner";
  }
  return inner();
}
print outer();           // expect: inner

// Functions as first-class values
fun identity(x) {
  return x;
}
print identity(42);      // expect: 42

fun apply(f, x) {
  return f(x);
}
print apply(identity, 99);  // expect: 99

// No arguments
fun zero() {
  return 0;
}
print zero();            // expect: 0

// Multiple parameters
fun sum3(a, b, c) {
  return a + b + c;
}
print sum3(1, 2, 3);     // expect: 6

// Recursive countdown
fun countdown(n) {
  if (n <= 0) {
    print "done";
    return;
  }
  print n;
  countdown(n - 1);
}
countdown(3);            // expect: 3
                         // expect: 2
                         // expect: 1
                         // expect: done
