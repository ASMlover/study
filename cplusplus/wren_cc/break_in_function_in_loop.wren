
var done = false
while (!done) {
  new Function {
    break // expect error
  }

  done = true
}
