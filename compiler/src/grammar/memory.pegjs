
MemoryInstruction = MemoryGet / MemorySet / MemoryCopy

MemoryCopy = destination:AddressValue ASSIGN source:AddressValue { return InstructionNode.create('memoryCopy', { source, destination }); }
MemoryGet = 'get' Spaces target:IdentifierValue Separator address:AddressValue { return InstructionNode.create('memoryGet', { target, address }); }
MemorySet = 'set' Spaces target:AddressValue Separator value:Value { return InstructionNode.create('memorySet', { target, value }); }
