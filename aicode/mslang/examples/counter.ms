// Closure-based counter pattern

fun makeCounter(start) {
  var count = start

  fun increment() {
    count = count + 1
    return count
  }

  fun decrement() {
    count = count - 1
    return count
  }

  fun value() {
    return count
  }

  // Return a "dispatch" closure
  fun counter(action) {
    if (action == "inc") return increment()
    if (action == "dec") return decrement()
    return value()
  }

  return counter
}

var c = makeCounter(0)
print c("val")     // 0
print c("inc")     // 1
print c("inc")     // 2
print c("inc")     // 3
print c("dec")     // 2
print c("val")     // 2

// Two independent counters
var a = makeCounter(10)
var b = makeCounter(100)
print a("inc")     // 11
print b("inc")     // 101
print a("inc")     // 12
print b("inc")     // 102
