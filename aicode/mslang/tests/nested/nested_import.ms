// Test: import from nested directory using relative path to sibling directory
import "../modules/math.ms";
print math.add(100, 200);       // expect: 300

from "../modules/utils.ms" import greet;
print greet("Nested");          // expect: Hello, Nested!
