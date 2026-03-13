class Root {
  init(name) {
    this.name = name;
  }

  label() {
    return "R:" + this.name;
  }
}

class Mid < Root {
  label() {
    return super.label() + "/M";
  }
}

class Leaf < Mid {
  label() {
    return super.label() + "/L";
  }
}

var item = Leaf("node");
print item.label();