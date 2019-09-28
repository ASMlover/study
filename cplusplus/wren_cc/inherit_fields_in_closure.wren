
class Foo {
  this new() { _field = "Foo field" }

  closeOverFooGet {
    return Function.new { Function.new { _field } }
  }

  closeOverFooSet {
    return Function.new { Function.new { _field = "new foo value" } }
  }
}

class Bar is Foo {
  this new() {
    super()
    _field = "Bar field"
  }

  closeOverBarGet {
    return Function.new { Function.new { _field } }
  }

  closeOverBarSet {
    return Function.new { Function.new { _field = "new bar value" } }
  }
}

var bar = Bar.new()
IO.print(bar.closeOverFooGet.call().call()) // expect: Foo field
IO.print(bar.closeOverBarGet.call().call()) // expect: Bar field
bar.closeOverFooSet.call().call()
IO.print(bar.closeOverFooGet.call().call()) // expect: new foo value
IO.print(bar.closeOverBarGet.call().call()) // expect: Bar field
bar.closeOverBarSet.call().call()
IO.print(bar.closeOverFooGet.call().call()) // expect: new foo value
IO.print(bar.closeOverBarGet.call().call()) // expect: new bar value
