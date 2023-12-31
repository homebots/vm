import {
  stringToHexBytes,
  bytesToNumber,
  numberToInt32,
  compile,
  stringToBytes,
  numberToUnsignedInt32,
  charArrayToBytes,
} from './index.mjs';

describe('data helpers', () => {
  describe('numberToInt32', () => {
    it('should convert a number to four bytes', () => {
      expect(numberToInt32(1000)).toEqual([232, 3, 0, 0]);
      expect(numberToInt32(-1000)).toEqual([24, 252, 255, 255]);
    });

    it('should throw error for numbers greater than 32 bits', () => {
      expect(() => numberToInt32(4294967296)).toThrow('Number is too large');
    });
  });

  describe('numberToUnsignedInt32', () => {
    it('should convert a number to four bytes', () => {
      expect(numberToUnsignedInt32(1000)).toEqual([232, 3, 0, 0]);
      expect(() => numberToUnsignedInt32(-1000)).toThrowError('Negative value not allowed');
    });

    it('should throw error for numbers greater than 32 bits', () => {
      expect(() => numberToUnsignedInt32(4294967296)).toThrow('Number is too large');
    });

    it('should throw error if number is negative', () => {
      expect(() => numberToUnsignedInt32(-1)).toThrow('Negative value not allowed');
    });
  });

  describe('bytesToNumber', () => {
    it('should convert int32 sequence of bytes to a number', () => {
      expect(bytesToNumber([232, 3, 0, 0])).toEqual(1000);
    });
  });

  describe('stringToHexBytes', () => {
    it('should convert a string into hex bytes', () => {
      expect(stringToHexBytes('0001ff00')).toBe('00 01 ff 00');
    });
  });

  describe('stringToBytes', () => {
    it('should convert a string into bytes', () => {
      expect(stringToBytes('foo')).toEqual([102, 111, 111, 0]);
    });
  });

  describe('charArrayToBytes', () => {
    it('should convert a string into bytes', () => {
      expect(charArrayToBytes(['f', 'o', 'o'])).toEqual([102, 111, 111, 0]);
    });
  });

  describe('identifiers', () => {
    it('should throw error if an identifier is redeclared', () => {
      expect(() =>
        compile(`
        byte $a = 0
        byte $a = 0
      `),
      ).toThrowError('Cannot redeclare identifier: $a');
    });

    it('should throw error if too many identifiers are declared', () => {
      const nodes = Array(256)
        .fill(0)
        .map((_, i) => 'byte $v' + i + ' = 0');

      const source = nodes.join('\n');

      expect(() => compile(source)).toThrowError('Too many identifiers');
    });
  });

  describe('types and length', () => {
    it('should calculate the size of data types correctly', () => {
      const program = `
      byte $a = ffh
      uint $b = 1
      int $c = +1
      string $d = 'hello'
      `;

      const bytes = compile(program);
      expect(bytes).toHaveLength(27);
    });
  });

  describe('variable declaration', () => {
    it('should throw an error if a declared variable does not match the initial value', () => {
      const program = `byte $a = 'foo'`;

      expect(() => compile(program)).toThrowError('Invalid value. Expected Byte but found String');
    });
  });
});
