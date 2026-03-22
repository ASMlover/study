enum Color { RED, GREEN, BLUE }
print Color.RED     // expect: 0
print Color.GREEN   // expect: 1
print Color.BLUE    // expect: 2

enum Status { OK = 200, NOT_FOUND = 404 }
print Status.OK         // expect: 200
print Status.NOT_FOUND  // expect: 404

switch (Color.GREEN) {
  case 0: print "red"
  case 1: print "green"   // expect: green
  default: print "other"
}
