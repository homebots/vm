import { OpCodes, CaptureOutput, compile, Emulator, StepClock } from '../index.mjs';

describe('operators', () => {
  function setup() {
    const emulator = new Emulator();
    const clock = new StepClock();
    const output = new CaptureOutput();
    return { emulator, clock, output };
  }

  it('should perform an arithmetic or logic operation', () => {
    const { emulator, clock, output } = setup();
    const bytes = compile(
      `
      uint $a = 1
      uint $b = 2
      uint $c = 4

      $a = $b + $c
      $a = $b - $c
      $a = $b * $c
      $a = $b / $c
      $a = $b and $c
      $a = $b or $c
      $a = $b xor $c
      $a = $b == $c
      $a = $b != $c
      $a = $b > $c
      $a = $b >= $c
      $a = $b < $c
      $a = $b <= $c
      $a = $b % $c
      $a = not $b
      $a = 1
      inc $a
      dec $a
      `,
    );

    const program = emulator.load(bytes, clock, output);

    const next = (expectedLine: string, expectedValue: unknown) => {
      clock.tick();
      const line = output.lines.shift() as string;

      if (!line) {
        throw new Error(`Expected ${expectedLine} but got nothing`);
      }

      expect(line).toBe(expectedLine);
      expect(program.variables[0].value).toBe(expectedValue);
    };

    next('declare #0, Integer, 1', 1);
    next('declare #1, Integer, 2', 1);
    next('declare #2, Integer, 4', 1);
    next(`#0 = #1 ${OpCodes.Add} #2`, 6);
    next(`#0 = #1 ${OpCodes.Sub} #2`, -2);
    next(`#0 = #1 ${OpCodes.Mul} #2`, 8);
    next(`#0 = #1 ${OpCodes.Div} #2`, 2 / 4);
    next(`#0 = #1 ${OpCodes.And} #2`, 2 & 4);
    next(`#0 = #1 ${OpCodes.Or} #2`, 2 | 4);
    next(`#0 = #1 ${OpCodes.Xor} #2`, 2 ^ 4);
    next(`#0 = #1 ${OpCodes.Equal} #2`, 0);
    next(`#0 = #1 ${OpCodes.NotEqual} #2`, 1);
    next(`#0 = #1 ${OpCodes.Gt} #2`, 0);
    next(`#0 = #1 ${OpCodes.Gte} #2`, 0);
    next(`#0 = #1 ${OpCodes.Lt} #2`, 1);
    next(`#0 = #1 ${OpCodes.Lte} #2`, 1);
    next(`#0 = #1 ${OpCodes.Mod} #2`, 2 % 4);
    next(`#0 = not #1`, 0);
    next(`#0 = 1`, 1);
    next(`inc #0`, 2);
    next(`dec #0`, 1);
  });

  it('should perform string operations', () => {
    const { emulator, clock, output } = setup();
    const bytes = compile(
      `
      string $a = ''
      $a = 'hello ' + 'world'
      `,
    );

    const program = emulator.load(bytes, clock, output);

    clock.tick(3);
    expect(program.variables[0].value).toBe('hello world');
  });
});
