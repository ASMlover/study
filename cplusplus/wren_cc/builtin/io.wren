
class IO {
  static print {
    IO.writeString("\n")
  }

  static print(obj) {
    IO.writeObject(obj)
    IO.writeString("\n")
    return obj
  }

  static print(a1, a2) {
    printList([a1, a2])
  }

  static print(a1, a2, a3) {
    printList([a1, a2, a3])
  }

  static print(a1, a2, a3, a4) {
    printList([a1, a2, a3, a4])
  }

  static print(a1, a2, a3, a4, a5) {
    printList([a1, a2, a3, a4, a5])
  }

  static print(a1, a2, a3, a4, a5, a6) {
    printList([a1, a2, a3, a4, a5, a6])
  }

  static print(a1, a2, a3, a4, a5, a6, a7) {
    printList([a1, a2, a3, a4, a5, a6, a7])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8) {
    printList([a1, a2, a3, a4, a5, a6, a7, a8])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9) {
    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) {
    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) {
    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) {
    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) {
    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) {
    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) {
    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15])
  }

  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) {
    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16])
  }

  static printList(objects) {
    for (object in objects) IO.writeObject(object)
    IO.writeString("\n")
  }

  static write(obj) {
    IO.writeObject(obj)
    return obj
  }

  static writeObject(obj) {
    var string = obj.toString
    if (string is String) {
      IO.writeString(string)
    } else {
      IO.writeString("[invalid toString]")
    }
  }
}
