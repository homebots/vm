import { bytesToNumber, OpCodes, ValueType } from '../compiler/index.mjs';
import { Clock, TimerClock } from './clock.mjs';
import { NullOutput, ProgramOutput } from './output.mjs';

const maximumSignedInteger = 2147483647;
const maximumUnsignedInteger = 4294967296;

const binaryOperatorTable: Record<number, (a: string | number, b: string | number) => unknown> = {
  [OpCodes.Gte]: (a, b) => Number(a >= b),
  [OpCodes.Gt]: (a, b) => Number(a > b),
  [OpCodes.Lte]: (a, b) => Number(a <= b),
  [OpCodes.Lt]: (a, b) => Number(a < b),
  [OpCodes.Equal]: (a, b) => Number(a === b),
  [OpCodes.NotEqual]: (a, b) => Number(a !== b),
  [OpCodes.Add]: (a, b) => (typeof a === 'string' ? a + b : Number(a) + Number(b)),
  [OpCodes.Sub]: (a: number, b: number) => a - b,
  [OpCodes.Mul]: (a: number, b: number) => a * b,
  [OpCodes.Div]: (a: number, b: number) => a / b,
  [OpCodes.Mod]: (a: number, b: number) => a % b,
  [OpCodes.Xor]: (a: number, b: number) => a ^ b,
  [OpCodes.And]: (a: number, b: number) => a & b,
  [OpCodes.Or]: (a: number, b: number) => a | b,
};
class Value {
  constructor(public dataType: ValueType, public value: string | number, public id?: number) {}

  isString() {
    return this.dataType === ValueType.String;
  }

  // isNumeric() {
  //   return !isNaN(this.toNumber());
  // }

  toString() {
    return String(this.value);
  }

  toNumber() {
    return Number(this.value);
  }

  toBoolean() {
    return Boolean(this.value);
  }
}

export class Program {
  constructor(readonly bytes: number[], readonly clock: Clock, readonly output: ProgramOutput) {
    this.endOfTheProgram = bytes.length;
    clock.onTick(() => this.tick());
  }

  counter = 0;
  pins: number[] = Array(16).fill(0);
  pinModes: number[] = Array(16).fill(0);
  pinTypes: number[] = Array(16).fill(0);
  variables: Value[] = Array(0xff);

  private endOfTheProgram = 0;

  tick(): void {
    const next = this.readByte();

    switch (next) {
      case OpCodes.Noop:
        this.trace('noop');
        break;

      case OpCodes.Halt:
        this.halt();
        break;

      case OpCodes.Print:
        this.print();
        break;

      case OpCodes.Delay:
        this.delay();
        break;

      case OpCodes.JumpTo:
        this.jumpTo();
        break;

      case OpCodes.Declare:
        this.declareIdentifier();
        break;

      case OpCodes.IoWrite:
        this.ioWrite();
        break;

      case OpCodes.IoMode:
        this.ioMode();
        break;

      case OpCodes.Not:
      case OpCodes.Inc:
      case OpCodes.Dec:
        this.unaryOperator(next);
        break;

      case OpCodes.Assign:
        this.assignOperator();
        break;

      case OpCodes.Gte:
      case OpCodes.Gt:
      case OpCodes.Lte:
      case OpCodes.Lt:
      case OpCodes.Equal:
      case OpCodes.NotEqual:
      case OpCodes.Add:
      case OpCodes.Sub:
      case OpCodes.Mul:
      case OpCodes.Div:
      case OpCodes.Mod:
      case OpCodes.Xor:
      case OpCodes.And:
      case OpCodes.Or:
        this.binaryOperator(next);
        break;

      default:
        throw new Error(`${this.counter}: Invalid opcode: ${next}`);
    }

    if (this.counter >= this.endOfTheProgram) {
      this.halt();
    }
  }

  move(amount: number): void {
    this.counter += amount;
  }

  trace(...args: unknown[]): void {
    this.output.trace(...args);
  }

  readByte(): number {
    const byte = this.bytes[this.counter];
    this.move(1);

    return byte;
  }

  readNumber(): number {
    const bytes = this.bytes.slice(this.counter, this.counter + 4);
    this.move(4);

    return bytesToNumber(bytes);
  }

  readString(): string {
    const string = [];

    while (this.bytes[this.counter] !== 0) {
      string.push(String.fromCharCode(this.bytes[this.counter]));
      this.move(1);
    }

    this.move(1);

    return string.join('');
  }

  readValue(): Value {
    const type: ValueType = this.readByte();
    let value: string | number;

    switch (type) {
      case ValueType.Byte:
      case ValueType.Pin:
      case ValueType.Identifier:
        value = this.readByte();
        break;

      case ValueType.Integer:
      case ValueType.Address:
        value = this.readNumber();
        break;

      case ValueType.SignedInteger:
        // [-2147483648 to 2147483647]
        value = this.readNumber();

        if (value > maximumSignedInteger) {
          value = (maximumUnsignedInteger - value) * -1;
        }

        break;

      case ValueType.String:
        value = this.readString();
        break;
    }

    if (type === ValueType.Identifier) {
      return this.variables[value];
    }

    return new Value(type, value);
  }

  halt(): void {
    this.trace('halt');
    this.clock.stop();
  }

  print(): void {
    const value = this.readValue();
    this.trace('print', value);
  }

  ioWrite(): void {
    const pin = this.readByte();
    const value = this.readValue();

    this.pins[pin] = value.toNumber();
    this.trace(`io write pin ${pin}, ${value}`);
  }

  ioMode(): void {
    const pin = this.readByte();
    const value = this.readByte();

    this.pinModes[pin] = value;
    this.trace(`io mode pin ${pin}, ${value}`);
  }

  delay(): void {
    const delay = this.readValue().toNumber();
    this.trace('delay', delay);

    this.clock.delay(delay);
  }

  jumpTo(): void {
    const position = this.readValue().toNumber();
    this.trace(`jump to ${position}`);
    this.counter = position;
  }

  declareIdentifier(): void {
    const id = this.readByte();
    const value = this.readValue();

    value.id = id;
    this.variables[id] = value;
    this.trace(`declare #${value.id}, ${ValueType[value.dataType]}, ${value.toString()}`);
  }

  notOperator(): void {
    const target = this.readValue();
    const value = this.readValue();

    this.trace(`#${target.id} = not #${value.id}`);
    target.value = Number(!value.toBoolean());
  }

  assignOperator(): void {
    const target = this.readValue();
    const value = this.readValue();

    this.trace(`#${target.id} = ${value.toString()}`);
    target.value = value.value;
  }

  unaryOperator(operator: number): void {
    switch (operator) {
      case OpCodes.Not:
        return this.notOperator();

      case OpCodes.Inc:
        return this.incOperator();

      case OpCodes.Dec:
        return this.decOperator();
    }
  }

  binaryOperator(operator: number): void {
    const fn = binaryOperatorTable[operator];
    const target = this.readValue();
    const a = this.readValue();
    const b = this.readValue();

    const valueOfA = a.isString() ? a.toString() : a.toNumber();
    const valueOfB = a.isString() ? b.toString() : b.toNumber();

    target.value = fn(valueOfA, valueOfB) as string | number;
    this.trace(`#${target.id} = #${a.id} ${operator} #${b.id}`);
  }

  incOperator(): void {
    const value = this.readValue();
    value.value = Number(value.value) + 1;
    this.trace('inc #' + value.id);
  }

  decOperator(): void {
    const value = this.readValue();
    value.value = Number(value.value) - 1;
    this.trace('dec #' + value.id);
  }
}

export class Emulator {
  load(bytes: number[], clock: Clock = new TimerClock(), output: ProgramOutput = new NullOutput()): Program {
    return new Program(bytes, clock, output);
  }
}
