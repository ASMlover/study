
class List {
  toString {
    var result = "["
    for (i in 0...len) {
      if (i > 0) result = result + ", "
      result = result + this[i].toString
    }
    result = result + "]"
    return result
  }
}
