fun greet(name, greeting = "Hello") {
  print greeting + ", " + name + "!"
}
greet("World")           // expect: Hello, World!
greet("World", "Hi")    // expect: Hi, World!

fun add(a, b = 10, c = 20) { return a + b + c; }
print add(1)        // expect: 31
print add(1, 2)     // expect: 23
print add(1, 2, 3)  // expect: 6
