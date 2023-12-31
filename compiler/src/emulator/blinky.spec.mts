import { CaptureOutput, compile, Emulator, StepClock } from '../index.mjs';

describe('Blinky program', () => {
  it.only('should run blinky', () => {
    const emulator = new Emulator();
    const clock = new StepClock();
    const output = new CaptureOutput();
    const bytes = compile(
      `
      // blinks a pin and loops back to zero
      // using tick() instead of run() to avoid infinite loop
      @begin
      byte $value = 0h
      io write pin 0, $value
      delay 1000
      $value = not $value
      io write pin 0, $value
      delay 1000
      jump to @begin
      `,
    );

    const program = emulator.load(bytes, clock, output);

    expect(program.counter).toBe(0);
    expect(program.pins[0]).toBe(0);

    clock.tick(7);

    expect(output.lines).toEqual([
      'declare #0, Byte, 0',
      'io write pin 0, 0',
      'delay 1000',
      '#0 = not #0',
      'io write pin 0, 1',
      'delay 1000',
      'jump to 0',
    ]);
  });
});
