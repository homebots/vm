export interface Clock {
  onTick(fn: () => void): void;
  start(): void;
  stop(): void;
  tick(): void;
  delay(delay: number): void;
}

export class StepClock implements Clock {
  steps: number;
  paused: boolean;

  constructor() {
    this.start();
  }

  start(): void {
    this.steps = 0;
    this.paused = false;
  }

  stop(): void {
    this.paused = true;
  }

  private fn: () => void;

  onTick(fn: () => void): void {
    this.fn = fn;
  }

  tick(times = 1): void {
    while (times-- && !this.paused) {
      this.fn();
      this.steps++;
    }
  }

  delay(_: number): void {
    // noop
  }

  run(): void {
    while (!this.paused) {
      this.tick(9999);
    }
  }
}

export class TimerClock implements Clock {
  private fn: () => void;
  private timer: unknown;
  paused = false;
  delayedBy = 0;

  onTick(fn: () => void): void {
    this.fn = fn;
  }

  start(): void {
    this.timer = setTimeout(() => this.tick(), 1);
    this.paused = false;
  }

  stop(): void {
    clearTimeout(this.timer as number);
    this.timer = 0;
    this.paused = true;
  }

  delay(delay: number): void {
    this.delayedBy = delay;
  }

  tick(): void {
    if (this.paused) {
      return;
    }

    if (this.delayedBy) {
      setTimeout(() => this.tick(), this.delayedBy);
      return;
    }

    this.fn();
    this.tick();
  }
}
