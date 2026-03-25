class Base {
  ping() {
    return "base"
  }
}

class Mid < Base {
  ping() {
    return super.ping() + "-mid"
  }
}

class Leaf < Mid {
  ping() {
    return super.ping() + "-leaf"
  }
}

print Leaf().ping()
