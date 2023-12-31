export const OpCodes = {
  // system instructions
  Noop: 0x01,
  Halt: 0x02,
  Restart: 0x03,
  SystemInfo: 0x04,
  Debug: 0x05,
  Dump: 0x06,
  Yield: 0x07,
  Delay: 0x08,
  Print: 0x09,
  JumpTo: 0x0a,
  JumpIf: 0x0b,
  Sleep: 0x0c,
  Declare: 0x0d,

  // operations
  Gt: 0x20,
  Gte: 0x21,
  Lt: 0x22,
  Lte: 0x23,
  Equal: 0x24,
  NotEqual: 0x25,
  Xor: 0x26,
  And: 0x27,
  Or: 0x28,
  Add: 0x29,
  Sub: 0x2a,
  Mul: 0x2b,
  Div: 0x2c,
  Mod: 0x2d,

  Not: 0x2e,
  Inc: 0x2f,
  Dec: 0x30,
  Assign: 0x31,

  // memory/io instructions
  MemGet: 0x40,
  MemSet: 0x41,
  MemCopy: 0x42,
  IoWrite: 0x43,
  IoRead: 0x44,
  IoMode: 0x45,
  IoType: 0x46,
  IoAllOut: 0x47,
};

export const binaryOperatorMap = {
  '>=': OpCodes.Gte,
  '>': OpCodes.Gt,
  '<=': OpCodes.Lte,
  '<': OpCodes.Lt,
  '==': OpCodes.Equal,
  '!=': OpCodes.NotEqual,
  '+': OpCodes.Add,
  '-': OpCodes.Sub,
  '*': OpCodes.Mul,
  '/': OpCodes.Div,
  '%': OpCodes.Mod,
  xor: OpCodes.Xor,
  and: OpCodes.And,
  or: OpCodes.Or,
};

export const unaryOperatorMap = {
  not: OpCodes.Not,
  inc: OpCodes.Inc,
  dec: OpCodes.Dec,
};
