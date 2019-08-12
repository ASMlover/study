
// single-expression body
for (i in [1]) IO.print(i)
// expect: 1

// block body
for (i in [1]) {
  IO.print(i)
}
// expect: 1

// newline after `in`
for (i in
  [1]) IO.print(i)
// expect: 1
