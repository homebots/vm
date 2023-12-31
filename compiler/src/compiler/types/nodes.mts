import { OpCodes, binaryOperatorMap, unaryOperatorMap } from './constants.mjs';
import { charArrayToBytes, numberToInt32, numberToUnsignedInt32 } from './data-conversion.mjs';

export interface NodeTypeToNodeMap {
  comment: InstructionNode;

  // values
  declareIdentifier: DeclareIdentifierNode;
  useIdentifier: UseIdentifierNode;
  defineLabel: LabelNode;
  label: LabelNode;

  byteValue: ByteValueNode;
  numberValue: NumberValueNode;
  stringValue: StringValueNode;
  identifierValue: IdentifierValueNode;

  // operators
  assign: AssignOperationNode;
  unaryOperation: UnaryOperationNode;
  binaryOperation: BinaryOperationNode;
  not: NotOperationNode;

  // system
  halt: InstructionNode;
  restart: InstructionNode;
  noop: InstructionNode;
  systemInfo: InstructionNode;
  dump: InstructionNode;
  print: NodeWithSingleValue<ValueNode<ValueNodePrimities>>;
  debug: NodeWithSingleValue<ValueNode<ValueNodePrimities>>;
  delay: NodeWithSingleValue<ValueNode<number>>;
  sleep: NodeWithSingleValue<ValueNode<number>>;
  yield: InstructionNode;
  jumpTo: SystemJumpToNode;
  jumpIf: SystemJumpIfNode;

  // io
  ioWrite: IoWriteNode;
  ioRead: IoReadNode;
  ioMode: IoModeNode;
  ioType: IoTypeNode;
  ioAllOut: InstructionNode;

  // memory
  memoryCopy: MemoryCopyNode;
  memoryGet: MemoryGetNode;
  memorySet: MemorySetNode;
}

// type NodeFactory<T extends InstructionNode> = (properties?: T) => T;
// const factories: { [K in keyof NodeTypeToNodeMap]?: NodeFactory<NodeTypeToNodeMap[K]> } = {};

type NodeSerializer<T extends InstructionNode> = (node: T) => Array<number | InstructionNode>;
let serializers: { [K in keyof NodeTypeToNodeMap]?: NodeSerializer<NodeTypeToNodeMap[K]> };

type NodeSizeOf<T extends InstructionNode> = (node: T) => number;
let sizeOf: { [K in keyof NodeTypeToNodeMap]?: NodeSizeOf<NodeTypeToNodeMap[K]> };

export class InstructionNode {
  type: keyof NodeTypeToNodeMap;

  static serialize(node: InstructionNode): Array<number | InstructionNode> | null {
    return (serializers[node.type] as NodeSerializer<InstructionNode>)(node);
  }

  static sizeOf(node: InstructionNode): number {
    return (sizeOf[node.type] as NodeSizeOf<InstructionNode>)(node);
  }

  static isOfType<K extends keyof NodeTypeToNodeMap>(item: InstructionNode, type: K): item is NodeTypeToNodeMap[K] {
    return Boolean(item && item.type === type);
  }

  static create<K extends keyof NodeTypeToNodeMap>(
    type: K,
    properties?: Omit<NodeTypeToNodeMap[K], 'type'>,
  ): NodeTypeToNodeMap[K] {
    properties = properties || ({} as NodeTypeToNodeMap[K]);

    // if (factories[type]) {
    //   const factory = factories[type];
    //   properties = factory(properties as never) as NodeTypeToNodeMap[K];
    // }

    return { ...properties, type } as NodeTypeToNodeMap[K];
  }
}

export enum ValueType {
  Null = 0,
  Identifier,
  Byte,
  Pin,
  Address,
  Integer,
  SignedInteger,
  String,
}

export interface NodeWithSingleValue<T> extends InstructionNode {
  value: T;
}

export type IdentifierType = ValueType.Byte | ValueType.Integer | ValueType.SignedInteger | ValueType.String;
export type ValueNodePrimities = number | string[];

export interface DeclareIdentifierNode<T extends ValueNodePrimities = ValueNodePrimities> extends InstructionNode {
  dataType: IdentifierType;
  value: ValueNode<T>;
  name: string;
  id?: number;
}

export interface UseIdentifierNode extends InstructionNode {
  name: string;
  id?: number;
}

export interface ValueNode<T = ValueNodePrimities | UseIdentifierNode> extends InstructionNode {
  dataType: ValueType;
  value: T;
}

export type ByteValueNode = ValueNode<number>;
export type NumberValueNode = ValueNode<number>;
export type StringValueNode = ValueNode<string[]>;
export type IdentifierValueNode = ValueNode<UseIdentifierNode>;

export interface IoWriteNode extends InstructionNode {
  pin: number;
  value: ValueNode;
}

export interface IoModeNode extends InstructionNode {
  pin: number;
  mode: number;
}

export interface IoTypeNode extends InstructionNode {
  pin: number;
  pinType: number;
}

export interface IoReadNode extends InstructionNode {
  pin: number;
  target: IdentifierValueNode;
}

export interface LabelNode extends InstructionNode {
  label: string;
}

export interface SystemJumpToNode extends InstructionNode {
  address?: NumberValueNode;
  label?: LabelNode;
}

export interface SystemJumpIfNode extends SystemJumpToNode {
  condition: ValueNode;
}

export interface MemorySetNode extends InstructionNode {
  target: NumberValueNode;
  value: ValueNode;
}

export interface MemoryGetNode extends InstructionNode {
  target: IdentifierValueNode;
  address: NumberValueNode;
}

export interface MemoryCopyNode extends InstructionNode {
  destination: NumberValueNode;
  source: NumberValueNode;
}

export interface UnaryOperationNode extends InstructionNode {
  operator: 'inc' | 'dec';
  target: IdentifierValueNode;
}

export interface BinaryOperationNode extends InstructionNode {
  operator: string;
  target: IdentifierValueNode;
  a: ValueNode;
  b: ValueNode;
}

export interface NotOperationNode extends InstructionNode {
  target: IdentifierValueNode;
  value: ValueNode;
}

export interface AssignOperationNode extends InstructionNode {
  target: IdentifierValueNode;
  value: ValueNode;
}

//////
export function valueToByteArray(type: ValueNode): number[] {
  switch (type.dataType) {
    case ValueType.Address:
    case ValueType.Integer:
      return numberToUnsignedInt32(type.value as number);

    case ValueType.SignedInteger:
      return numberToInt32(type.value as number);

    case ValueType.Byte:
    case ValueType.Pin:
      return [type.value as number];

    case ValueType.Identifier:
      return [(type.value as UseIdentifierNode).id];

    case ValueType.String:
      return charArrayToBytes(type.value as unknown as string[]);
  }
}

export function serializeValue(value: ValueNode): number[] {
  return [value.dataType].concat(valueToByteArray(value));
}

serializers = {
  comment: () => [],
  declareIdentifier: (node) => [OpCodes.Declare, node.id, ...serializeValue(node.value)],
  halt: () => [OpCodes.Halt],
  restart: () => [OpCodes.Restart],
  noop: () => [OpCodes.Noop],
  systemInfo: () => [OpCodes.SystemInfo],
  dump: () => [OpCodes.Dump],
  debug: (node) => [OpCodes.Debug, ...serializeValue(node.value)],
  delay: (node) => [OpCodes.Delay, ...serializeValue(node.value)],
  print: (node) => [OpCodes.Print, ...serializeValue(node.value)],
  sleep: (node) => [OpCodes.Sleep, ...serializeValue(node.value)],
  yield: () => [OpCodes.Yield],
  assign: (node) => [OpCodes.Assign, ...serializeValue(node.target), ...serializeValue(node.value)],
  not: (node) => [OpCodes.Not, ...serializeValue(node.target), ...serializeValue(node.value)],
  unaryOperation: (node) => [unaryOperatorMap[node.operator], ...serializeValue(node.target)],
  binaryOperation: (node) => [
    binaryOperatorMap[node.operator],
    ...serializeValue(node.target),
    ...serializeValue(node.a),
    ...serializeValue(node.b),
  ],
  jumpTo: (node) => [OpCodes.JumpTo, ...serializeValue(node.address)],
  jumpIf: (node) => [OpCodes.JumpIf, ...serializeValue(node.condition), ...serializeValue(node.address)],
  ioMode: (node) => [OpCodes.IoMode, node.pin, node.mode],
  ioType: (node) => [OpCodes.IoType, node.pin, node.pinType],
  ioAllOut: () => [OpCodes.IoAllOut],
  ioWrite: (node) => [OpCodes.IoWrite, node.pin, ...serializeValue(node.value)],
  ioRead: (node) => [OpCodes.IoRead, node.pin, ...serializeValue(node.target)],
  memoryGet: (node) => [OpCodes.MemGet, ...serializeValue(node.target), ...serializeValue(node.address)],
  memorySet: (node) => [OpCodes.MemSet, ...serializeValue(node.target), ...serializeValue(node.value)],
  memoryCopy: (node) => [OpCodes.MemCopy, ...serializeValue(node.source), ...serializeValue(node.destination)],
};

const oneByte = () => 1;

sizeOf = {
  comment: () => 0,

  // values
  declareIdentifier: (node) => 2 + serializeValue(node.value).length,
  useIdentifier: () => 0,
  defineLabel: () => 0,
  label: () => 0,
  stringValue: (node) => serializeValue(node).length,
  byteValue: (node) => serializeValue(node).length,
  numberValue: (node) => serializeValue(node).length,
  identifierValue: (node) => serializeValue(node).length,

  // io
  ioWrite: (node) => 2 + serializeValue(node.value).length,
  ioRead: (node) => 2 + serializeValue(node.target).length,
  ioMode: () => 3,
  ioType: () => 3,
  ioAllOut: oneByte,

  // system
  halt: oneByte,
  restart: oneByte,
  noop: oneByte,
  systemInfo: oneByte,
  dump: oneByte,
  debug: (node) => 1 + serializeValue(node.value).length,
  print: (node) => 1 + serializeValue(node.value).length,
  delay: (node) => 1 + serializeValue(node.value).length,
  sleep: (node) => 1 + serializeValue(node.value).length,
  yield: oneByte,

  jumpTo: () => 6,
  jumpIf: (node) => 6 + serializeValue(node.condition).length,

  // operators
  assign: (node) => 1 + serializeValue(node.target).length + serializeValue(node.value).length,
  unaryOperation: (node) => 1 + serializeValue(node.target).length,
  binaryOperation: (node) =>
    1 + serializeValue(node.target).length + serializeValue(node.a).length + serializeValue(node.b).length,
  not: oneByte,

  // memory
  memoryGet: (node) => 1 + serializeValue(node.target).length + serializeValue(node.address).length,
  memorySet: (node) => 1 + serializeValue(node.target).length + serializeValue(node.value).length,
  memoryCopy: () => 11,
};
