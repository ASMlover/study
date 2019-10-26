
{
  var a = "a"
  if (false) Function.new { a }
}

IO.print("ok") // expect: ok
