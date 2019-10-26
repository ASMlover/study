
{
  // basic assignment

  var list = [1, 2, 3]
  list[0] = 11
  list[1] = 22
  list[2] = 33
  IO.print(list) // expect: [11, 22, 33]
}

{
  // returns right-hand side

  var list = [1, 2, 3]
  IO.print(list[1] = 22) // expect: 22
}

{
  // negative indices

  var list = [1, 2, 3]
  list[-1] = 11
  list[-2] = 22
  list[-3] = 33
  IO.print(list) // expect: [33, 22, 11]
}
