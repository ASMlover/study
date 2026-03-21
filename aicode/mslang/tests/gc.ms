// Test: Generational GC — young objects collected, old survive promotion

// 1. Allocate many short-lived objects to trigger minor GC
var sum = 0
for (var i = 0; i < 1000; i = i + 1) {
  var temp = [i, i + 1, i + 2]
  sum = sum + temp[0]
}
print sum // expect: 499500

// 2. Long-lived objects survive promotion
class Node {
  init(val) {
    this.val = val
    this.next = nil
  }
}

var head = Node(0)
var curr = head
for (var i = 1; i < 100; i = i + 1) {
  curr.next = Node(i)
  curr = curr.next
}

// Walk list to verify integrity after GC cycles
var count = 0
var walker = head
while (walker != nil) {
  count = count + 1
  walker = walker.next
}
print count // expect: 100

// 3. Closures capturing upvalues survive across GC
fun make_adder(x) {
  return fun(y) { return x + y; }
}

var add5 = make_adder(5)
var add10 = make_adder(10)

// Trigger more allocation between captures
for (var i = 0; i < 500; i = i + 1) {
  var junk = "garbage_" + str(i)
}

print add5(3)  // expect: 8
print add10(7) // expect: 17

// 4. Write barrier: old object referencing young object
class Container {
  init() {
    this.items = []
  }
}

var c = Container()
// Let container age
for (var i = 0; i < 500; i = i + 1) {
  var junk = [i]
}
// Now store fresh young objects into the (potentially promoted) container
for (var i = 0; i < 10; i = i + 1) {
  c.items.push(i)
}
print c.items.len() // expect: 10

// 5. Map with many entries
var m = {}
for (var i = 0; i < 200; i = i + 1) {
  m[str(i)] = i * i
}
print m["99"] // expect: 9801
print m["0"]  // expect: 0
