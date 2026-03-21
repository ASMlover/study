// Basic closure: capture outer variable
print "--- capture outer variable ---" // expect: --- capture outer variable ---
fun makeGreeter(greeting) {
  fun greet(name) {
    print greeting + ", " + name + "!"
  }
  return greet
}
var hello = makeGreeter("Hello")
hello("World")              // expect: Hello, World!
hello("Maple")              // expect: Hello, Maple!

// Counter pattern
print "--- counter pattern ---" // expect: --- counter pattern ---
fun makeCounter() {
  var count = 0
  fun increment() {
    count = count + 1
    return count
  }
  return increment
}
var counter = makeCounter()
print counter()             // expect: 1
print counter()             // expect: 2
print counter()             // expect: 3

// Independent counters
print "--- independent counters ---" // expect: --- independent counters ---
var c1 = makeCounter()
var c2 = makeCounter()
print c1()                  // expect: 1
print c2()                  // expect: 1
print c1()                  // expect: 2
print c2()                  // expect: 2

// Closed-over variable persists after outer returns
print "--- upvalue persists after return ---" // expect: --- upvalue persists after return ---
fun makeAdder(x) {
  fun adder(y) {
    return x + y
  }
  return adder
}
var add5 = makeAdder(5)
print add5(10)              // expect: 15
print add5(20)              // expect: 25

// Shared upvalues: two closures capture the same variable
print "--- shared upvalues ---" // expect: --- shared upvalues ---
fun shared() {
  var value = "original"
  fun getter() {
    return value
  }
  fun setter(v) {
    value = v
  }
  print getter()            // expect: original
  setter("changed")
  print getter()            // expect: changed
}
shared()

// Nested closures
print "--- nested closures ---" // expect: --- nested closures ---
fun outer() {
  var x = "outer"
  fun middle() {
    fun inner() {
      print x
    }
    inner()
  }
  middle()
}
outer()                     // expect: outer

// Closure captures variable, not value
print "--- capture variable not value ---" // expect: --- capture variable not value ---
fun captureLoop() {
  var fns = nil
  var i = 0
  // Each iteration creates a new scope with a new local
  while (i < 3) {
    var value = i
    fun capture() {
      print value
    }
    // Only keep last one
    fns = capture
    i = i + 1
  }
  fns()
}
captureLoop()               // expect: 2

// Upvalue in deeply nested function
print "--- deeply nested upvalue ---" // expect: --- deeply nested upvalue ---
fun a() {
  var x = "a"
  fun b() {
    fun c() {
      fun d() {
        print x
      }
      d()
    }
    c()
  }
  b()
}
a()                         // expect: a
