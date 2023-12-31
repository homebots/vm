

// operators
Operator = BinaryOperation / UnaryOperation / DeclareIdentifier / Assign

ASSIGN = '='

Assign = target:IdentifierValue Spaces ASSIGN Spaces value:Value { return InstructionNode.create('assign', { target, value }) }

UnaryOperation = not / step
not = target:IdentifierValue Spaces ASSIGN Spaces 'not' Spaces value:Value { return InstructionNode.create('not', { target, value }) }
step = operator:('inc' / 'dec') Spaces target:IdentifierValue { return InstructionNode.create('unaryOperation', { operator, target }) }

BinaryOperation = target:IdentifierValue Spaces ASSIGN Spaces a:Value Spaces operator:BinaryOperator Spaces b:Value { return InstructionNode.create('binaryOperation', { operator, target, a, b }) }
BinaryOperator = '>=' / '>' / '<=' / '<' / '==' / '!=' / 'xor' / 'and' / 'or' / '+' / '-' / '*' / '/' / '%'

DeclareIdentifier = dataType:ValueTypeMap Spaces name:Identifier Spaces ASSIGN Spaces value:Value { return InstructionNode.create('declareIdentifier', { name, dataType, value }) }

// values
IdentifierValue = value:UseIdentifier { return InstructionNode.create('identifierValue', { value, dataType: ValueType.Identifier }) }
PinValue =  value:Pin { return InstructionNode.create('byteValue', { value, dataType: ValueType.Pin }) }
BooleanValue = value:Boolean { return InstructionNode.create('byteValue', { value, dataType: ValueType.Byte }) }
ByteValue = value:Byte { return InstructionNode.create('byteValue', { value, dataType: ValueType.Byte }) }
AddressValue = value:Address { return InstructionNode.create('numberValue', { value, dataType: ValueType.Address }) }
IntegerValue = value:Integer { return InstructionNode.create('numberValue', { value, dataType: ValueType.Integer }) }
SignedIntegerValue = value:SignedInteger { return InstructionNode.create('numberValue', { value, dataType: ValueType.SignedInteger }) }
StringValue = value:String { return InstructionNode.create('stringValue', { value, dataType: ValueType.String }) }
NullValue = 'null' { return InstructionNode.create('byteValue', { value: 0, dataType: ValueType.Null }) }
NumberValue = IntegerValue / SignedIntegerValue
Value "value" = IdentifierValue / ByteValue / AddressValue / NumberValue / StringValue / BooleanValue / NullValue
IntrinsicValue = ByteValue / NumberValue / StringValue / BooleanValue / NullValue

SystemInstruction 'system instruction' = Halt / Restart / SystemInfo / Debug / Dump / Noop / Print / JumpTo / JumpIf / Delay

Halt = 'halt' { return InstructionNode.create('halt') }
Restart = 'restart' { return InstructionNode.create('restart') }
Noop = 'noop' { return InstructionNode.create('noop') }
SystemInfo = 'sysinfo' { return InstructionNode.create('systemInfo') }
Dump = 'dump' { return InstructionNode.create('dump') }
Debug = 'debug' Spaces value:BooleanValue { return InstructionNode.create('debug', { value }) }
Print = 'print' Spaces value:Value { return InstructionNode.create('print', { value }) }

Delay =
  'delay' Spaces value:IntegerValue { return InstructionNode.create('delay', { value }) } /
  'sleep' Spaces value:IntegerValue { return InstructionNode.create('sleep', { value }) } /
  'yield' { return InstructionNode.create('yield') }

JumpTo =
  'jump' Spaces 'to' Spaces address:AddressValue { return InstructionNode.create('jumpTo', { address }) } /
  'jump' Spaces 'to' Spaces '@' label:Label { return InstructionNode.create('jumpTo', { label }) }

JumpIf =
  'if' Spaces condition:Value Spaces 'then' Spaces 'jump' Spaces  'to' Spaces address:AddressValue { return InstructionNode.create('jumpIf', { condition, address }) } /
  'if' Spaces condition:Value Spaces 'then' Spaces 'jump' Spaces  'to' Spaces '@' label:Label { return InstructionNode.create('jumpIf', { condition, label }) }

