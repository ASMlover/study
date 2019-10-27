
import "random" for Random

var random = Random.new(12345)

// empty list
var list = []
random.shuffle(list)
IO.print(list) // expect: []

// one list
list = [1]
random.shuffle(list)
IO.print(list) // expect: [1]

// given enough tries, should generate all permutations
var hits = {}
for (i in 1..200) {
  var list = [1, 2, 3, 4]
  random.shuffle(list)
  hits[list.toString] = true
}
IO.print(hits.len) // expect: 24
