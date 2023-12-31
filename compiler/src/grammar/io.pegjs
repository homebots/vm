
IoInstruction
  = IoWrite / IoRead / IoMode / IoType / IoAllOut

IoWrite = 'io write' Spaces pin:Pin Separator value:IoValue { return InstructionNode.create('ioWrite', { pin, value }) }
IoRead = 'io read' Spaces target:IdentifierValue Separator pin:Pin { return InstructionNode.create('ioRead', { pin, target }) }
IoMode = 'io mode' Spaces pin:Pin Separator mode:PinMode { return InstructionNode.create('ioMode', { pin, mode }) }
IoType = 'io type' Spaces pin:Pin Separator pinType:Digit { return InstructionNode.create('ioType', { pin, pinType: Number(pinType) }) }
IoAllOut = 'io all out' { return InstructionNode.create('ioAllOut') }

IoValue = ByteValue / IdentifierValue / BooleanValue