import { compile, StepClock, Emulator, CaptureOutput } from '../index.mjs';

describe('prints back to serial output', () => {
  it('should print a string', () => {
    const program = `print 'hello world!'`;
    const output = new CaptureOutput();
    const emulator = new Emulator();
    const stepper = new StepClock();
    const bytes = compile(program);

    emulator.load(bytes, stepper, output);
    stepper.run();

    expect(output.lines).toEqual(['print hello world!', 'halt']);
  });
});
