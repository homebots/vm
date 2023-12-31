export interface ProgramOutput {
  trace(...args: unknown[]): void;
}

export class NullOutput implements ProgramOutput {
  trace(..._args: unknown[]): void {
    return;
  }
}

export class LogOutput implements ProgramOutput {
  trace(...args: unknown[]): void {
    console.log(...args);
  }
}

export class CaptureOutput implements ProgramOutput {
  readonly logs: string[][] = [];
  readonly lines: string[] = [];

  trace(...args: unknown[]): void {
    const line = args.map(String);

    this.logs.push(line);
    this.lines.push(line.join(' '));
  }
}
