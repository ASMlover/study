// Global variables
print "--- global variables ---" // expect: --- global variables ---
var a = 10
print a             // expect: 10

var b
print b             // expect: nil

b = 20
print b             // expect: 20

// Assignment returns the value
print "--- assignment expression ---" // expect: --- assignment expression ---
var c = a + b
print c             // expect: 30

// Local variables and scoping
print "--- local variables and scoping ---" // expect: --- local variables and scoping ---
{
  var x = "local"
  print x           // expect: local
}

// Shadowing
print "--- variable shadowing ---" // expect: --- variable shadowing ---
var d = "outer"
{
  var d = "inner"
  print d           // expect: inner
}
print d             // expect: outer

// Nested scopes
print "--- nested scopes ---" // expect: --- nested scopes ---
{
  var e = 1
  {
    var f = 2
    print e + f     // expect: 3
  }
}
