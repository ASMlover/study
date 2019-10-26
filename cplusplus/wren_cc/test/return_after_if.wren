
IO.print(Function.new {
  if (true) return "ok"
}.call()) // expect: ok
