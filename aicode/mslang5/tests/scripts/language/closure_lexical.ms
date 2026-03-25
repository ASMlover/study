var x = "global"
fun outer() {
  var x = "local"
  fun read() {
    return x
  }
  return read
}

var fn = outer()
print fn()
print x
