
class IO {
  static print {
    writeString("\n")
  }

  static print(obj) {
    writeObject(obj)
    writeString("\n")
    return obj
  }

  static print(a1, a2) {
    printAll([a1, a2])
  }

  static print(a1, a2, a3) {
    printAll([a1, a2, a3])
  }

  static print(a1, a2, a3, a4) {
    printAll([a1, a2, a3, a4])
  }

  static print(a1, a2, a3, a4, a5) {
    printAll([a1, a2, a3, a4, a5])
  }

  static print(a1, a2, a3, a4, a5, a6) {
    printAll([a1, a2, a3, a4, a5, a6])
  }

  static print(a1, a2, a3, a4, a5, a6, a7) {
    printAll([a1, a2, a3, a4, a5, a6, a7])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8) {
    printAll([a1, a2, a3, a4, a5, a6, a7, a8])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9) {
    printAll([a1, a2, a3, a4, a5, a6, a7, a8, a9])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) {
    printAll([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) {
    printAll([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) {
    printAll([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) {
    printAll([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) {
    printAll([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) {
    printAll([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) {
    printAll([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16])
  }

  static printAll(sequence) {
    for (object in sequence) writeObject(object)
    writeString("\n")
  }

  static write(obj) {
    writeObject(obj)
    return obj
  }

  static writeAll(sequence) {
    for (object in sequence) writeObject(object)
  }

  static read(prompt) {
    if (!(prompt is String)) Fiber.abort("Prompt must be a string")
    write(prompt)
    return read()
  }

  static writeObject(obj) {
    var string = obj.toString
    if (string is String) {
      writeString(string)
    } else {
      writeString("[invalid toString]")
    }
  }

  foreign static writeString(string)
  foreign static clock
  foreign static time
  foreign static read()
}
