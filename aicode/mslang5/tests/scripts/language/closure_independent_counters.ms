fun makeCounter(seed) {
  var n = seed
  fun inc() {
    n = n + 1
    return n
  }
  return inc
}

var c1 = makeCounter(0)
var c2 = makeCounter(100)
print c1()
print c1()
print c2()
print c1()
print c2()

