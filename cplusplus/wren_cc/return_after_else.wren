
IO.print(new Function {
  if (false) "no" else return "ok"
}.call()) // expect: ok
