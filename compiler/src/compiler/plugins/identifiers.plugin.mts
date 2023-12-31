import { CompilationContext, CompilerPlugin } from '../compiler.mjs';
import { DeclareIdentifierNode, InstructionNode, UseIdentifierNode, ValueType } from '../types/index.mjs';

export class FindIdentifiersPlugin implements CompilerPlugin {
  run(context: CompilationContext): CompilationContext {
    const { nodes, identifiers, identifierTypes } = context;

    nodes.forEach((node) => {
      if (InstructionNode.isOfType(node, 'declareIdentifier')) {
        this.declareIdentifier(identifiers, identifierTypes, node);
      }
    });

    return context;
  }

  private declareIdentifier(
    identifiers: Map<string, number>,
    identifierTypes: Map<string, ValueType>,
    node: DeclareIdentifierNode,
  ) {
    if (identifiers.size >= 0xff) {
      throw new Error('Too many identifiers');
    }

    if (!identifiers.has(node.name)) {
      node.id = identifiers.size;
      identifiers.set(node.name, identifiers.size);
      identifierTypes.set(node.name, node.dataType);
      return;
    }

    throw new Error('Cannot redeclare identifier: ' + node.name);
  }
}

export class ReplaceIdentifiersPlugin implements CompilerPlugin {
  run(context: CompilationContext): CompilationContext {
    context.nodes.forEach((node) => this.walkNode(context, node));

    return context;
  }

  private walkNode(context: CompilationContext, node: InstructionNode): void {
    if (InstructionNode.isOfType(node, 'useIdentifier')) {
      this.replaceIdentifier(context, node);
      return;
    }

    Object.keys(node).forEach((key) => {
      if (typeof node[key] === 'object' && node[key] !== null) {
        this.walkNode(context, node[key]);
      }
    });
  }

  private replaceIdentifier(context: CompilationContext, node: UseIdentifierNode) {
    if (!context.identifiers.has(node.name)) {
      throw new Error(`Identifier not found: ${node.name}`);
    }

    node.id = context.identifiers.get(node.name);
  }
}
