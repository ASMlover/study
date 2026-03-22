fun sum(...args) {
  var total = 0
  for (var x in args) { total = total + x }
  return total
}
print sum()           // expect: 0
print sum(1)          // expect: 1
print sum(1, 2, 3)    // expect: 6
print sum(1, 2, 3, 4) // expect: 10
