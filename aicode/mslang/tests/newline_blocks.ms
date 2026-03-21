// Test: block / class / function: `}` ASI, cross-line declarations

// --- function declarations ---
print "--- functions ---" // expect: --- functions ---
fun hello() {
  return "hi"
}
print hello() // expect: hi

fun multiply(a, b) {
  return a * b
}
print multiply(3, 4) // expect: 12

// --- consecutive function declarations ---
print "--- consecutive funs ---" // expect: --- consecutive funs ---
fun first() {
  return 1
}

fun second() {
  return 2
}

fun third() {
  return 3
}
print first() + second() + third() // expect: 6

// --- nested functions ---
print "--- nested funs ---" // expect: --- nested funs ---
fun outer() {
  fun inner() {
    return "inner"
  }
  return inner()
}
print outer() // expect: inner

// --- block scoping ---
print "--- block scope ---" // expect: --- block scope ---
var a = "outer"
{
  var a = "inner"
  print a // expect: inner
}
print a // expect: outer

// --- consecutive blocks ---
print "--- consecutive blocks ---" // expect: --- consecutive blocks ---
{
  print "block1" // expect: block1
}
{
  print "block2" // expect: block2
}

// --- class declaration ---
print "--- class ---" // expect: --- class ---
class Animal {
  init(name) {
    this.name = name
  }

  speak() {
    return this.name + " speaks"
  }
}
var dog = Animal("Dog")
print dog.speak() // expect: Dog speaks

// --- class with multiple methods ---
print "--- class methods ---" // expect: --- class methods ---
class Counter {
  init() {
    this.count = 0
  }

  increment() {
    this.count += 1
  }

  value() {
    return this.count
  }
}
var c = Counter()
c.increment()
c.increment()
c.increment()
print c.value() // expect: 3

// --- class inheritance ---
print "--- inheritance ---" // expect: --- inheritance ---
class Base {
  greet() {
    return "base"
  }
}

class Derived : Base {
  greet() {
    return "derived:" + super.greet()
  }
}
print Derived().greet() // expect: derived:base

// --- consecutive class declarations ---
print "--- consecutive classes ---" // expect: --- consecutive classes ---
class A {
  value() {
    return "A"
  }
}

class B {
  value() {
    return "B"
  }
}
print A().value() + B().value() // expect: AB

// --- class after function ---
print "--- class after fun ---" // expect: --- class after fun ---
fun make_point(x, y) {
  return [x, y]
}

class Box {
  init(w, h) {
    this.w = w
    this.h = h
  }

  area() {
    return this.w * this.h
  }
}
var p = make_point(1, 2)
print p // expect: [1, 2]
print Box(3, 4).area() // expect: 12

// --- try-catch blocks ---
print "--- try-catch ---" // expect: --- try-catch ---
try {
  throw "oops"
} catch (e) {
  print e // expect: oops
}

// --- if with blocks after } ---
print "--- if blocks ---" // expect: --- if blocks ---
if (true) {
  print "yes" // expect: yes
}
print "after if" // expect: after if

if (false) {
  print "no"
} else {
  print "else" // expect: else
}
print "after else" // expect: after else
