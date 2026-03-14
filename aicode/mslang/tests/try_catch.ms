// Test 1: Basic try/catch with throw
try {
  throw "error!";
  print "should not reach here";
} catch (e) {
  print e;  // expect: error!
}

// Test 2: Try block without exception
try {
  print "no error";  // expect: no error
} catch (e) {
  print "should not catch";
}

// Test 3: Throw non-string value
try {
  throw 42;
} catch (e) {
  print e;  // expect: 42
}

// Test 4: Nested try/catch
try {
  try {
    throw "inner";
  } catch (e) {
    print e;  // expect: inner
  }
  print "outer continues";  // expect: outer continues
} catch (e) {
  print "should not reach outer catch";
}

// Test 5: Rethrow from catch
try {
  try {
    throw "rethrown";
  } catch (e) {
    throw e;
  }
} catch (e) {
  print e;  // expect: rethrown
}

// Test 6: Catch runtime error (undefined variable)
try {
  print undefined_var;
} catch (e) {
  print e;  // expect: Undefined variable 'undefined_var'.
}

// Test 7: Exception across function call
fun risky() {
  throw "from function";
}
try {
  risky();
} catch (e) {
  print e;  // expect: from function
}

// Test 8: Exception across nested function calls
fun a() { throw "deep"; }
fun b() { a(); }
fun c() { b(); }
try {
  c();
} catch (e) {
  print e;  // expect: deep
}

// Test 9: Code after try/catch continues normally
try {
  throw "handled";
} catch (e) {
  print e;  // expect: handled
}
print "after try/catch";  // expect: after try/catch

// Test 10: Catch runtime error from division or operations
try {
  var x = nil + 1;
} catch (e) {
  print e;  // expect: Operands must be two numbers or two strings.
}
