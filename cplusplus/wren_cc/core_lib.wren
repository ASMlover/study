
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
    for (i in 0...this.len) {
      if (i > 0) result = result + ", "
      result = result + this[i].toString
    }
    result = result + "]"
    return result
  }
}
