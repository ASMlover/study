// Test calling abstract method raises runtime error
class Animal {
  abstract speak() {}
}

var a = Animal();
a.speak(); // expect runtime error
