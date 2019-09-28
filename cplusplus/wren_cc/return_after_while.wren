
IO.print(Function.new {
  while (true) return "ok"
}.call()) // expect: ok
