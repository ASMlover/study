
import "meta" for Meta

var variables = Meta.getModuleVariables("main")

IO.print(variables.contains("Object")) // expect: true
IO.print(variables.contains("Bool")) // expect: true
IO.print(variables.contains("variables")) // expect: true

IO.print(variables.contains("later")) // expect: true
var later = "values"

IO.print(variables.contains("unknown")) // expect: false
