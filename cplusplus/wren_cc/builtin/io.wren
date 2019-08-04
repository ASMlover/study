
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
