// Test: import statement
import "tests/modules/math.ms";
print math.add(1, 2);           // expect: 3
print math.multiply(3, 4);      // expect: 12
print math.PI;                  // expect: 3.14159

// Test: from ... import
from "tests/modules/utils.ms" import greet;
print greet("Maple");           // expect: Hello, Maple!

// Test: from ... import ... as
from "tests/modules/utils.ms" import double as dbl;
print dbl(21);                  // expect: 42

// Test: from ... import (another name from same module)
from "tests/modules/math.ms" import add as plus;
print plus(10, 20);             // expect: 30
