import { compile } from '../compiler/index.mjs';
import { StepClock } from './clock.mjs';
import { Emulator } from './emulator.mjs';

describe('noop', () => {
  it('should run one instruction and halt program', async () => {
    const emulator = new Emulator();
    const stepper = new StepClock();
    const bytes = compile(`
    noop
    halt
    `);

    const program = emulator.load(bytes, stepper);
    expect(program.counter).toBe(0);

    stepper.tick();
    expect(program.counter).toBe(1);

    stepper.tick();
    expect(program.counter).toBe(2);
  });

  it('should stop program if there are no further instruction', async () => {
    const emulator = new Emulator();
    const stepper = new StepClock();
    const bytes = compile(
      `noop
       noop`,
    );

    const program = emulator.load(bytes, stepper);
    expect(program.counter).toBe(0);

    stepper.run();

    expect(program.counter).toBe(2);
    expect(stepper.paused).toBe(true);
  });
});
