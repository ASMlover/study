// Anonymous function assigned to variable
var double = fun(x) { return x * 2; };
print double(5);             // expect: 10

// Lambda passed as argument
fun apply(f, x) { return f(x); }
print apply(fun(x) { return x + 1; }, 9);  // expect: 10

// Lambda with no params
var greet = fun() { return "hello"; };
print greet();               // expect: hello

// Immediately invoked
print fun(a, b) { return a + b; }(3, 4);   // expect: 7

// Lambda capturing upvalue (closure)
fun make_adder(n) {
  return fun(x) { return x + n; };
}
var add5 = make_adder(5);
print add5(10);              // expect: 15

// Lambda in list
var fns = [fun(x) { return x * x; }];
print fns[0](6);             // expect: 36
