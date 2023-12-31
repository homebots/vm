import { compile } from '../compiler/index.mjs';
import { StepClock } from './clock.mjs';
import { Emulator } from './emulator.mjs';
import { CaptureOutput } from './output.mjs';

describe('data types', () => {
  it('should initialise variables with different data types', () => {
    const program = `
    byte $a = 1h
    byte $b = true
    int $minusOne = -1
    int $lowestInt = -2147483648
    int $int = -2147483647
    int $positiveInt = +2147483647
    uint $e = 123
    string $f = 'hello'
    address $g = 0x00543f01
    `;

    const output = new CaptureOutput();
    const emulator = new Emulator();
    const stepper = new StepClock();
    const bytes = compile(program);

    emulator.load(bytes, stepper, output);

    stepper.run();

    expect(output.lines).toEqual([
      'declare #0, Byte, 1',
      'declare #1, Byte, 1',
      'declare #2, SignedInteger, -1',
      'declare #3, SignedInteger, -2147483648',
      'declare #4, SignedInteger, -2147483647',
      'declare #5, SignedInteger, 2147483647',
      'declare #6, Integer, 123',
      'declare #7, String, hello',
      'declare #8, Address, 5521153',

      'halt',
    ]);
  });

  it('should support pin modes as string or number', () => {
    const program = `
    io mode pin 1, input
    io mode pin 1, output
    io mode pin 1, open-drain
    io mode pin 1, pull-up

    io mode pin 1, 0
    io mode pin 1, 1
    io mode pin 1, 2
    io mode pin 1, 3
    `;

    const output = new CaptureOutput();
    const emulator = new Emulator();
    const stepper = new StepClock();
    const bytes = compile(program);

    emulator.load(bytes, stepper, output);

    stepper.run();

    expect(output.lines).toEqual([
      'io mode pin 1, 0',
      'io mode pin 1, 1',
      'io mode pin 1, 2',
      'io mode pin 1, 3',

      'io mode pin 1, 0',
      'io mode pin 1, 1',
      'io mode pin 1, 2',
      'io mode pin 1, 3',
      'halt',
    ]);
  });
});
