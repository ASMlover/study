// StringBuilder: amortized O(1) append for efficient string building

// Basic creation and build
{
  var sb = StringBuilder();
  sb.append("hello");
  sb.append(" ");
  sb.append("world");
  print sb.build();  // expect: hello world
}

// Creation with initial value
{
  var sb = StringBuilder("start");
  sb.append("_end");
  print sb.build();  // expect: start_end
}

// Chaining (append returns the builder)
{
  var sb = StringBuilder();
  sb.append("a").append("b").append("c");
  print sb.build();  // expect: abc
}

// len() method
{
  var sb = StringBuilder();
  print sb.len();    // expect: 0
  sb.append("hello");
  print sb.len();    // expect: 5
  sb.append("!");
  print sb.len();    // expect: 6
}

// clear() method
{
  var sb = StringBuilder();
  sb.append("hello");
  sb.clear();
  print sb.len();    // expect: 0
  sb.append("world");
  print sb.build();  // expect: world
}

// Appending non-string values (auto-stringify)
{
  var sb = StringBuilder();
  sb.append(42);
  sb.append(true);
  sb.append(nil);
  print sb.build();  // expect: 42truenil
}

// type() check
{
  var sb = StringBuilder();
  print type(sb);    // expect: stringbuilder
}

// Loop building (the main use case — O(n) instead of O(n^2))
{
  var sb = StringBuilder();
  for (var i = 0; i < 5; i += 1) {
    sb.append(i);
    if (i < 4) sb.append(",");
  }
  print sb.build();  // expect: 0,1,2,3,4
}

// Build can be called multiple times (non-destructive)
{
  var sb = StringBuilder();
  sb.append("test");
  print sb.build();  // expect: test
  sb.append("!");
  print sb.build();  // expect: test!
}

// Clear then reuse
{
  var sb = StringBuilder();
  sb.append("first");
  sb.clear();
  sb.append("second");
  print sb.build();  // expect: second
}

// StringBuilder in a function
fun join(list, sep) {
  var sb = StringBuilder();
  for (var i = 0; i < list.len(); i += 1) {
    if (i > 0) sb.append(sep);
    sb.append(list[i]);
  }
  return sb.build();
}
print join(["a", "b", "c"], "-");  // expect: a-b-c

// Large string building
{
  var sb = StringBuilder();
  for (var i = 0; i < 100; i += 1) {
    sb.append("x");
  }
  print sb.len();  // expect: 100
}
