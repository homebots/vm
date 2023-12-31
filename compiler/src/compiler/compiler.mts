import { parse } from './parser.mjs';
import { InstructionNode, ValueType } from './types/index.mjs';

export type ByteArray = Array<number>;

export interface CompilationContext {
  nodes: InstructionNode[];
  bytes: ByteArray;
  identifiers: Map<string, number>;
  identifierTypes: Map<string, ValueType>;
  labelAddresses: Map<string, number>;
}

export interface CompilerPlugin {
  run(context: CompilationContext): CompilationContext;
}

interface ParseError {
  location: { start: { line: number; column: number } };
  message: string;
}

export class Compiler {
  parse(code: string): Array<InstructionNode> {
    code = code.trim();

    if (!code) {
      return [];
    }

    return parse(code).flat(2);
  }

  compile<P extends CompilerPlugin[]>(code: Array<InstructionNode>, plugins?: P): ByteArray;
  compile<P extends CompilerPlugin[]>(code: string, plugins?: P): ByteArray;
  compile<P extends CompilerPlugin[]>(code: string | Array<InstructionNode>, plugins?: P): ByteArray {
    try {
      const nodes: Array<InstructionNode> = typeof code === 'string' ? this.parse(code) : code;
      const bytes = this.runPlugins(nodes, plugins);

      return bytes;
    } catch (error) {
      this.handleError(error, String(code).trim());
    }
  }

  protected handleError<T extends Error>(error: ParseError | T, source: string): void {
    if ('location' in error) {
      const { line, column } = error.location.start;
      const message =
        line +
        ':' +
        column +
        ': ' +
        error.message +
        '\n' +
        source.split('\n')[error.location.start.line - 1] +
        '\n' +
        ' '.repeat(column - 1) +
        '^';

      throw new SyntaxError(message);
    }

    throw error;
  }

  protected runPlugins<P extends CompilerPlugin[]>(nodes: Array<InstructionNode>, plugins: P): ByteArray {
    const initialContext = this.createCompilationContext(nodes);
    const context = plugins.reduce((context, plugin) => plugin.run(context), initialContext);

    // console.log(JSON.stringify(context.nodes, null, 2));
    // console.log(context.bytes);

    return context.bytes as ByteArray;
  }

  protected createCompilationContext(nodes: Array<InstructionNode>): CompilationContext {
    const identifiers = new Map<string, number>();
    const identifierTypes = new Map<string, ValueType>();
    const labelAddresses = new Map<string, number>();

    return {
      bytes: [],
      nodes,
      identifiers,
      identifierTypes,
      labelAddresses,
    };
  }
}
