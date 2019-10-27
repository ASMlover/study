
import "random" for Random

var random = Random.new(12345)

// single element list
IO.print(random.sample(["single"], 1)) // expect: [single]

// should choose all elements with roughly equal probability
var list = ["a", "b", "c", "d", "e"]
var histogram = {}
for (i in 1..5000) {
  var sample = random.sample(list, 1)
  var string = sample.toString
  if (!histogram.containsKey(string)) histogram[string] = 0
  histogram[string] = histogram[string] + 1
}

IO.print(histogram.len) // expect: 5
for (key in histogram.keys) {
  var error = (histogram[key] / (5000 / list.len) - 1).abs
  if (error > 0.1) IO.print("!!! " + error.toString + " !!!")
}
