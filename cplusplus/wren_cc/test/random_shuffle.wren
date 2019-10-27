
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
var histogram = {}
for (i in 1..5000) {
  var list = [1, 2, 3, 4]
  random.shuffle(list)

  var string = list.toString
  if (!histogram.containsKey(string)) histogram[string] = 0
  histogram[string] = histogram[string] + 1
}
IO.print(histogram.len) // expect: 24
for (key in histogram.keys) {
  var error = (histogram[key] / (5000 / 24) - 1).abs
  if (error > 0.2) IO.print("!!! " + error.toString + " !!!")
}
