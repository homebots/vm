import { InstructionNode, NodeTypeToNodeMap, OpCodes, ValueType } from './types/index.mjs';

describe('InstructionNode.sizeOf and InstructionNode.serialize', () => {
  const createIdentifier = (id: number) =>
    InstructionNode.create('identifierValue', {
      value: InstructionNode.create('useIdentifier', { name: 'foo', id }),
      dataType: ValueType.Identifier,
    });

  describe('should calculate the instruction size correctly:', () => {
    it('assign', () => {
      const bytes = 'foo'.split('').map((c) => c.charCodeAt(0));
      const node = InstructionNode.create('assign', {
        target: createIdentifier(2),
        value: InstructionNode.create('stringValue', { value: ['f', 'o', 'o'], dataType: ValueType.String }),
      });

      expect(InstructionNode.sizeOf(node)).toBe(8);
      expect(InstructionNode.serialize(node)).toEqual([
        OpCodes.Assign,
        ValueType.Identifier,
        2,
        ValueType.String,
        ...bytes,
        0,
      ]);
    });

    it('binaryOperation', () => {
      const node = InstructionNode.create('binaryOperation', {
        a: InstructionNode.create('byteValue', { value: 1, dataType: ValueType.Byte }),
        b: InstructionNode.create('byteValue', { value: 2, dataType: ValueType.Byte }),
        operator: '+',
        target: createIdentifier(2),
      });

      expect(InstructionNode.sizeOf(node)).toBe(7);
      expect(InstructionNode.serialize(node)).toEqual([
        OpCodes.Add,
        ValueType.Identifier,
        2,
        ValueType.Byte,
        1,
        ValueType.Byte,
        2,
      ]);
    });

    it('unaryOperation', () => {
      const node = InstructionNode.create('unaryOperation', {
        target: createIdentifier(0),
        operator: 'inc',
      });
      expect(InstructionNode.sizeOf(node)).toBe(3);
      expect(InstructionNode.serialize(node)).toEqual([OpCodes.Inc, ValueType.Identifier, 0]);
    });

    it('comment', () => {
      const node = InstructionNode.create('comment', {});
      expect(InstructionNode.sizeOf(node)).toBe(0);
      expect(InstructionNode.serialize(node)).toEqual([]);
    });

    it('declareIdentifier', () => {
      const node = InstructionNode.create('declareIdentifier', {
        dataType: ValueType.Byte,
        name: 'foo',
        id: 1,
        value: InstructionNode.create('byteValue', { value: 0, dataType: ValueType.Byte }),
      });

      expect(InstructionNode.sizeOf(node)).toBe(4);
      expect(InstructionNode.serialize(node)).toEqual([OpCodes.Declare, 1, ValueType.Byte, 0]);
    });

    it('useIdentifier', () => {
      const node = InstructionNode.create('useIdentifier', { name: 'foo' });

      expect(InstructionNode.sizeOf(node)).toBe(0);
      expect(() => InstructionNode.serialize(node)).toThrow();
    });

    it('defineLabel', () => {
      const node = InstructionNode.create('defineLabel', { label: 'foo' });

      expect(InstructionNode.sizeOf(node)).toBe(0);
      expect(() => InstructionNode.serialize(node)).toThrow();
    });

    it('label', () => {
      const node = InstructionNode.create('label', { label: 'foo' });

      expect(InstructionNode.sizeOf(node)).toBe(0);
      expect(() => InstructionNode.serialize(node)).toThrow();
    });

    it('stringValue', () => {
      const node = InstructionNode.create('stringValue', { value: 'hello'.split(''), dataType: ValueType.String });

      expect(InstructionNode.sizeOf(node)).toBe(7);
      expect(() => InstructionNode.serialize(node)).toThrow();
    });

    it('numberValue', () => {
      const node = InstructionNode.create('numberValue', { value: 1000, dataType: ValueType.Integer });

      expect(InstructionNode.sizeOf(node)).toBe(5);
      expect(() => InstructionNode.serialize(node)).toThrow();
    });

    it('byteValue', () => {
      const node = InstructionNode.create('byteValue', { value: 1, dataType: ValueType.Byte });

      expect(InstructionNode.sizeOf(node)).toBe(2);
      expect(() => InstructionNode.serialize(node)).toThrow();
    });

    it('identifierValue', () => {
      const node = createIdentifier(0);

      expect(InstructionNode.sizeOf(node)).toBe(2);
      expect(() => InstructionNode.serialize(node)).toThrow();
    });

    it('debug', () => {
      const byte = InstructionNode.create('byteValue', { value: 1, dataType: ValueType.Byte });
      const node = InstructionNode.create('debug', { value: byte });

      expect(InstructionNode.sizeOf(node)).toBe(3);
      expect(InstructionNode.serialize(node)).toEqual([OpCodes.Debug, ValueType.Byte, 1]);
    });

    it('single-byte instructions', () => {
      const instructionTypes: Array<keyof NodeTypeToNodeMap> = ['halt', 'restart', 'noop', 'systemInfo', 'dump'];

      instructionTypes.forEach((type) => expect(InstructionNode.sizeOf(InstructionNode.create(type))).toBe(1));

      expect(InstructionNode.serialize(InstructionNode.create('halt'))).toEqual([OpCodes.Halt]);
      expect(InstructionNode.serialize(InstructionNode.create('restart'))).toEqual([OpCodes.Restart]);
      expect(InstructionNode.serialize(InstructionNode.create('noop'))).toEqual([OpCodes.Noop]);
      expect(InstructionNode.serialize(InstructionNode.create('systemInfo'))).toEqual([OpCodes.SystemInfo]);
      expect(InstructionNode.serialize(InstructionNode.create('dump'))).toEqual([OpCodes.Dump]);
    });

    it('print', () => {
      const node = InstructionNode.create('print', {
        value: InstructionNode.create('numberValue', { value: 1000, dataType: ValueType.Integer }),
      });

      expect(InstructionNode.sizeOf(node)).toBe(6);
      expect(InstructionNode.serialize(node)).toEqual([OpCodes.Print, ValueType.Integer, 0xe8, 0x03, 0, 0]);
    });

    const time = {
      value: InstructionNode.create('numberValue', { value: 1000, dataType: ValueType.Integer }),
    };

    it('delay', () => {
      const node = InstructionNode.create('delay', time);
      expect(InstructionNode.sizeOf(node)).toBe(6);
      expect(InstructionNode.serialize(node)).toEqual([OpCodes.Delay, ValueType.Integer, 0xe8, 0x03, 0, 0]);
    });

    it('sleep', () => {
      const node = InstructionNode.create('sleep', time);
      expect(InstructionNode.sizeOf(node)).toBe(6);
      expect(InstructionNode.serialize(node)).toEqual([OpCodes.Sleep, ValueType.Integer, 0xe8, 0x03, 0, 0]);
    });

    it('yield', () => {
      const node = InstructionNode.create('yield');
      expect(InstructionNode.sizeOf(node)).toBe(1);
      expect(InstructionNode.serialize(node)).toEqual([OpCodes.Yield]);
    });

    it('jumpTo', () => {
      const node = InstructionNode.create('jumpTo', {
        address: InstructionNode.create('numberValue', { value: 1000, dataType: ValueType.Address }),
      });
      expect(InstructionNode.sizeOf(node)).toBe(6);
      expect(InstructionNode.serialize(node)).toEqual([OpCodes.JumpTo, ValueType.Address, 0xe8, 0x03, 0, 0]);
    });

    it('jumpIf', () => {
      const value = InstructionNode.create('byteValue', { value: 1, dataType: ValueType.Byte });
      const node = InstructionNode.create('jumpIf', {
        condition: value,
        address: InstructionNode.create('numberValue', { value: 1000, dataType: ValueType.Address }),
      });

      expect(InstructionNode.sizeOf(node)).toBe(8);
      expect(InstructionNode.serialize(node)).toEqual([
        OpCodes.JumpIf,
        ValueType.Byte,
        1,
        ValueType.Address,
        0xe8,
        0x03,
        0,
        0,
      ]);
    });

    it('ioWrite', () => {
      expect(
        InstructionNode.sizeOf(
          InstructionNode.create('ioWrite', {
            pin: 1,
            value: InstructionNode.create('byteValue', { value: 1, dataType: ValueType.Byte }),
          }),
        ),
      ).toBe(4);
    });

    it('ioRead', () => {
      const node = InstructionNode.create('ioRead', {
        pin: 1,
        target: createIdentifier(1),
      });

      expect(InstructionNode.sizeOf(node)).toBe(4);
      expect(InstructionNode.serialize(node)).toEqual([OpCodes.IoRead, 1, ValueType.Identifier, 1]);
    });

    it('ioMode', () => {
      const node = InstructionNode.create('ioMode', { pin: 1, mode: 2 });

      expect(InstructionNode.sizeOf(node)).toBe(3);
      expect(InstructionNode.serialize(node)).toEqual([OpCodes.IoMode, 1, 2]);
    });

    it('ioType', () => {
      const node = InstructionNode.create('ioType', { pin: 1, pinType: 2 });

      expect(InstructionNode.sizeOf(node)).toBe(3);
      expect(InstructionNode.serialize(node)).toEqual([OpCodes.IoType, 1, 2]);
    });

    it('ioAllOut', () => {
      const node = InstructionNode.create('ioAllOut');

      expect(InstructionNode.sizeOf(node)).toBe(1);
      expect(InstructionNode.serialize(node)).toEqual([OpCodes.IoAllOut]);
    });

    it('memoryGet', () => {
      const node = InstructionNode.create('memoryGet', {
        address: InstructionNode.create('numberValue', { dataType: ValueType.Address, value: 1000 }),
        target: createIdentifier(1),
      });

      expect(InstructionNode.sizeOf(node)).toBe(8);
      expect(InstructionNode.serialize(node)).toEqual([
        OpCodes.MemGet,
        ValueType.Identifier,
        1,
        ValueType.Address,
        0xe8,
        0x03,
        0,
        0,
      ]);
    });

    it('memorySet', () => {
      const node = InstructionNode.create('memorySet', {
        target: InstructionNode.create('numberValue', { dataType: ValueType.Address, value: 1000 }),
        value: InstructionNode.create('numberValue', { dataType: ValueType.Integer, value: 1001 }),
      });

      expect(InstructionNode.sizeOf(node)).toBe(11);
      expect(InstructionNode.serialize(node)).toEqual([
        OpCodes.MemSet,
        ValueType.Address,
        0xe8,
        0x03,
        0,
        0,
        ValueType.Integer,
        0xe9,
        0x03,
        0,
        0,
      ]);
    });

    it('memoryCopy', () => {
      const node = InstructionNode.create('memoryCopy', {
        source: InstructionNode.create('numberValue', { dataType: ValueType.Address, value: 1000 }),
        destination: InstructionNode.create('numberValue', { dataType: ValueType.Address, value: 1001 }),
      });

      expect(InstructionNode.sizeOf(node)).toBe(11);
      expect(InstructionNode.serialize(node)).toEqual([
        OpCodes.MemCopy,
        ValueType.Address,
        0xe8,
        0x03,
        0,
        0,
        ValueType.Address,
        0xe9,
        0x03,
        0,
        0,
      ]);
    });
  });
});
