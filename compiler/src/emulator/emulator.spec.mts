import { LogOutput, Emulator, StepClock, compile, TimerClock } from '../index.mjs';

describe('vm emulator', () => {
  it('should throw an error for invalid instructions', async () => {
    const emulator = new Emulator();
    const stepper = new StepClock();
    const bytes = [0];

    emulator.load(bytes, stepper);

    expect(() => stepper.tick()).toThrow('Invalid opcode: 0');
  });

  it('should load bytes into a program and return it', async () => {
    const emulator = new Emulator();
    const bytes = compile(`noop`);
    const program = emulator.load(bytes);

    expect(program.counter).toBe(0);
    expect(program.bytes).toEqual(bytes);
  });

  it('should run the program with a real clock', async () => {
    const emulator = new Emulator();
    const clock = new TimerClock();
    const bytes = compile(
      `
      noop
      delay 5
      noop
      halt`,
    );

    const program = emulator.load(bytes, clock);
    expect(program.counter).toBe(0);

    clock.start();

    await new Promise((resolve) => setTimeout(resolve, 50));

    expect(program.counter).toBe(7);
    clock.stop();

    expect(clock.paused).toBe(true);
    clock.tick();

    expect(clock.paused).toBe(true);
    expect(program.counter).toBe(7);
  });

  it('should log steps', async () => {
    const emulator = new Emulator();
    const stepper = new StepClock();
    const bytes = compile(`noop`);
    const logger = jest.spyOn(console, 'log').mockImplementation(() => 0);

    emulator.load(bytes, stepper, new LogOutput());
    stepper.run();

    expect(logger).toHaveBeenCalledWith('noop');
  });
});
