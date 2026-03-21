// Test: inherit from non-class
var NotAClass = 42
class Bad : NotAClass {} // expect runtime error: Superclass must be a class.
