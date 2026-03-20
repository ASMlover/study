class Root {
  init(name) {
    this.name = name
  }

  label() {
    return "R:" + this.name
  }
}

class Mid < Root {
  init(name) {
    super.init(name)
  }

  label() {
    return super.label() + "/M"
  }
}

class Leaf < Mid {
  init(name) {
    super.init(name)
  }

  label() {
    return super.label() + "/L"
  }
}

var item = Leaf("node")
print item.label()