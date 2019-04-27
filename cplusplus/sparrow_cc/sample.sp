
// sample of sparrow

import person for Person

fun fn() {
  var p = Person.new("Tom", "male")
  p.say()
}

class Family < Person {
  var father
  var mother
  var child

  new(f, m, c) {
    father = f
    mother = m
    child = c
    super("Jack", "male")
  }
}

var f = Family.new("Jack", "Lily", "Buddy")
f.say()

fn()
