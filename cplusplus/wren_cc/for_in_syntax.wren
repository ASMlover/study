
// single-expression body
for (i in [1, 2, 3]) IO.print(i)
// expect: 1
// expect: 2
// expect: 3

// block body
for (i in [1, 2, 3]) {
  IO.print(i)
}
// expect: 1
// expect: 2
// expect: 3

// newline after `for`
for
(i in [1, 2, 3]) IO.print(i)
// expect: 1
// expect: 2
// expect: 3
