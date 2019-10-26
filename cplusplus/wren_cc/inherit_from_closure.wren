
var ClosureType

{
  var a = "a"
  ClosureType = Function.new { IO.print(a) }.type
}

class SubClass is ClosureType {} // expect runtime error
