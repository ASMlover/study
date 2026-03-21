// Test 1: Basic defer — executes at function return
fun test_basic() {
  defer print "deferred";
  print "body";
}
test_basic();
// expect: body
// expect: deferred

// Test 2: LIFO order — last defer runs first
fun test_lifo() {
  defer print "first";
  defer print "second";
  defer print "third";
  print "body";
}
test_lifo();
// expect: body
// expect: third
// expect: second
// expect: first

// Test 3: Defer with return value — defer runs, return value preserved
fun test_return() {
  defer print "cleanup";
  return "result";
}
print test_return();
// expect: cleanup
// expect: result

// Test 4: Defer in top-level scope
defer print "top-level defer";
print "top-level body";
// expect: top-level body

// Test 5: Multiple defers with interleaved code
fun test_interleaved() {
  print "a";
  defer print "d1";
  print "b";
  defer print "d2";
  print "c";
}
test_interleaved();
// expect: a
// expect: b
// expect: c
// expect: d2
// expect: d1

// Test 6: Defer captures variables (closure)
fun test_closure() {
  var x = "hello";
  defer print x;
  x = "world";
}
test_closure();
// expect: world

// Test 7: Defer with explicit return in middle
fun test_early_return() {
  defer print "always runs";
  if (true) {
    return;
  }
  print "never reached";
}
test_early_return();
// expect: always runs

// Test 8: Nested function defers are independent
fun outer() {
  defer print "outer-defer";
  fun inner() {
    defer print "inner-defer";
    print "inner-body";
  }
  inner();
  print "outer-body";
}
outer();
// expect: inner-body
// expect: inner-defer
// expect: outer-body
// expect: outer-defer
// expect: top-level defer
