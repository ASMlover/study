
class Sequence {
  count {
    var result = 0
    for (element in this) {
      result = result + 1
    }
    return result
  }

  map(f) {
    var result = new List
    for (element in this) {
      result.add(f.call(element))
    }
    return result
  }

  where(f) {
    var result = new List
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

  any(f) {
    for (element in this) {
      if (f.call(element)) return true
    }
    return false
  }

  reduce(acc, f) {
    for (element in this) {
      acc = f.call(acc, element)
    }
    return acc
  }

  reduce(f) {
    var iter = iterate(nil)
    if (!iter) Fiber.abort("cannot reduce an empty sequence")

    // seed with the first element
    var result = iterValue(iter)
    while (iter = iterate(iter)) {
      result = f.call(result, iterValue(iter))
    }
    return result
  }

  join { join("") }

  join(sep) {
    var first = true
    var result = ""

    for (element in this) {
      if (!first) result = result + sep
      first = false
      result = result + element.toString
    }
    return result
  }
}

class String is Sequence {}

class List is Sequence {
  addAll(other) {
    for (element in other) {
      add(element)
    }
    return other
  }

  toString { "[" + join(", ") + "]" }

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

class MapKeySequence is Sequence {
  new(map) {
    _map = map
  }

  iterate(n) { _map.iter(n) }
  iterValue(iterator) { _map.keyIterValue(iterator) }
}

class MapValSequence is Sequence {
  new(map) {
    _map = map
  }

  iterate(n) { _map.iter(n) }
  iterValue(iterator) { _map.valIterValue(iterator) }
}

class Map {
  keys { new MapKeySequence(this) }
  values { new MapValSequence(this) }

  toString {
    var first = true
    var result = "{"

    for (key in keys) {
      if (!first) result = result + ", "
      first = false
      result = result + key.toString + ": " + this[key].toString
    }

    return result + "}"
  }
}
