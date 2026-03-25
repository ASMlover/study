fun makeValue() {
  var seed = 40
  fun addTwo() {
    return seed + 2
  }
  return addTwo()
}

print makeValue()
