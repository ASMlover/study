
class IO {
  static write(obj) {
    IO.write__native__(obj.toString)
    return obj
  }
}

class List {
  toString {
    var result = "["
    var i = 0
    // TODO: use for loop
    while (i < this.len) {
      if (i > 0) result = result + ", "
      result = result + this[i].toString
      i = i + 1
    }
    result = result + "]"
    return result
  }
}

class Range {
  new(min, max) {
    _min = min
    _max = max
  }

  min { return _min }
  max { return _max }

  iterate(prev) {
    if (prev == nil) return _min
    if (prev == _max) return false
    return prev + 1
  }

  iterValue(i) {
    return i
  }
}

class Numeric {
  .. other { return new Range(this, other) }
  ... other { return new Range(this, other - 1) }
}
