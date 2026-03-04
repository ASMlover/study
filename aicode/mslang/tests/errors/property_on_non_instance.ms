// Test: property access on non-instance
var x = 42;
x.field; // expect runtime error: Only instances have properties.
