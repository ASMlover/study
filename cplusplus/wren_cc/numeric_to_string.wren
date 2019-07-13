
IO.write(123.toString)
IO.write(-123.toString)
IO.write(-0.toString)

IO.write(12.34.toString == "12.34") // expect: true
IO.write(-0.0001.toString == "-0.0001") // expect: true
