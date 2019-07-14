
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
