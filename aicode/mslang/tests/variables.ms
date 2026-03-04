// Global variables
var a = 10;
print a;             // expect: 10

var b;
print b;             // expect: nil

b = 20;
print b;             // expect: 20

// Assignment returns the value
var c = a + b;
print c;             // expect: 30

// Local variables and scoping
{
  var x = "local";
  print x;           // expect: local
}

// Shadowing
var d = "outer";
{
  var d = "inner";
  print d;           // expect: inner
}
print d;             // expect: outer

// Nested scopes
{
  var e = 1;
  {
    var f = 2;
    print e + f;     // expect: 3
  }
}
