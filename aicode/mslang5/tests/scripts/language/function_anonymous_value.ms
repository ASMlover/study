var sum3 = (fun(a, b, c) {
  return a + b + c
})

var wrap = (fun(x) {
  return (fun(y) {
    return x + y
  })
})

var add10 = wrap(10)
print sum3(1, 2, 3)
print add10(5)