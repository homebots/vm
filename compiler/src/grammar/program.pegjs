
Program = c:Line* { return c }
Line "statement" = Spaces c:Statement StatementSeparator { return c }
StatementSeparator = NewLine? / ';'?
Statement "statement" =
DefineLabel /
SystemInstruction /
MemoryInstruction /
Operator /
IoInstruction /
Comment

 // / WifiInstruction
 // / I2cInstruction


Comment "comment" = ('//' [^\n]+) { return [] }