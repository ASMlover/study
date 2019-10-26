
// allow after `{`, `:` and `,`, and before `]`
var map = {

"a":

"a value",

"b": "b value"

}

IO.print(map["a"]) // expect: a value
IO.print(map["b"]) // expect: b value

// newline after trailing comma
map = {"c": "c value",

}
IO.print(map["c"]) // expect: c value

// newline in empty map
map = {

}
IO.print(map.len) // expect: 0
