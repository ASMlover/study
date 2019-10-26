
for (n in 1.3..4.5) IO.print(n)
// expect: 1.3
// expect: 2.3
// expect: 3.3
// expect: 4.3

for (n in 1.3...4.5) IO.print(n)
// expect: 1.3
// expect: 2.3
// expect: 3.3
// expect: 4.3

for (n in 1.3...4.3) IO.print(n)
// expect: 1.3
// expect: 2.3
// expect: 3.3
