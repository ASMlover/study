
IO.print(Function.new {
  if (false) "no" else return "ok"
}.call()) // expect: ok
