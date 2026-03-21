// Test basic modulo
print "--- basic modulo ---" // expect: --- basic modulo ---
print 10 % 3 // expect: 1
print 7 % 2 // expect: 1
print 8 % 4 // expect: 0

// Test modulo with larger numbers
print "--- modulo with larger numbers ---" // expect: --- modulo with larger numbers ---
print 100 % 7 // expect: 2

// Test modulo precedence (same as * and /)
print "--- modulo precedence ---" // expect: --- modulo precedence ---
print 2 + 10 % 3 // expect: 3
print 10 % 3 + 2 // expect: 3

// Test modulo with floating point
print "--- modulo with float ---" // expect: --- modulo with float ---
print 10.5 % 3 // expect: 1.5

// Test chained modulo
print "--- chained modulo ---" // expect: --- chained modulo ---
print 17 % 5 % 3 // expect: 2
