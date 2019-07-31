
class IO {
  static print(obj) {
    IO.writeString(obj.toString)
    IO.writeString("\n")
    return obj
  }

  static write(obj) {
    IO.writeString(obj.toString)
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
