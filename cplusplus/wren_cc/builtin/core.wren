
class Sequence {
  map(f) {
    var result = []
    for (element in this) {
      result.add(f.call(element))
    }
    return result
  }

  where(f) {
    var result = []
    for (element in this) {
      if (f.call(element)) result.add(element)
    }
    return result
  }

  all(f) {
    for (element in this) {
      if (!f.call(element)) return false
    }
    return true
  }

  reduce(acc, f) {
    for (element in this) {
      acc = f.call(acc, element)
    }
    return acc
  }

  reduce(f) { this[1..-1].reduce(this[0], f) }
}

class List is Sequence {
  addAll(other) {
    for (element in other) {
      add(element)
    }
    return other
  }

  toString {
    var result = "["
    for (i in 0...len) {
      if (i > 0) result = result + ", "
      result = result + this[i].toString
    }
    result = result + "]"
    return result
  }

  +(other) {
    var result = this[0..-1]
    for (element in other) {
      result.add(element)
    }
    return result
  }

  contains(element) {
    for (item in this) {
      if (element == item) {
        return true
      }
    }
    return false
  }
}

class Range is Sequence {}
