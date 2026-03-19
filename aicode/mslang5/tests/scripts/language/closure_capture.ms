fun makeCounter(start) {
  var value = start
  fun inc() {
    value = value + 1
    return value
  }
  return inc
}

var c = makeCounter(10)
print c()
print c()
