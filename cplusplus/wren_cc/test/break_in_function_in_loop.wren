
var done = false
while (!done) {
  Function.new {
    break // expect error
  }

  done = true
}
