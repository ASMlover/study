package main

import "fmt"

func swap(x, y string) (string, string) {
	return y, x
}

func main() {
	x, y := swap("Hello", "world")
	fmt.Println(x, y)
}
