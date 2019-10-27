
import "random" for Random

var random = Random.new(12345)

// single element list
IO.print(random.sample(["single"], 1)) // expect: [single]

// should choose all elements with roughly equal probability
var list = ["a", "b", "c", "d", "e"]
var histogram = {"a":0, "b":0, "c":0, "d":0, "e":0}
for (i in 1..1000) {
  var sample = random.sample(list)
  histogram[sample] = histogram[sample] + 1
}

IO.print(histogram.len) // expect: 5
for (key in histogram.keys) {
  var error = (histogram[key] / (1000 / list.len) - 1).abs
  if (error > 0.2) IO.print("!!! " + error.toString + " !!!")
}
