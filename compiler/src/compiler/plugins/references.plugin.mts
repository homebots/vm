import { CompilationContext, CompilerPlugin } from '../compiler.mjs';
import { InstructionNode, NumberValueNode, ValueType } from '../types/index.mjs';

export class FindLabelsPlugin implements CompilerPlugin {
  run(context: CompilationContext): CompilationContext {
    const labelAddresses = new Map<string, number>();
    let byteAccumulator = 0;

    const nodes = context.nodes.filter((node) => {
      if (InstructionNode.isOfType(node, 'defineLabel')) {
        const position = byteAccumulator - labelAddresses.size;
        labelAddresses.set(node.label, position);

        return false;
      }

      byteAccumulator += InstructionNode.sizeOf(node);

      return true;
    });

    return {
      ...context,
      labelAddresses,
      nodes,
    };
  }
}

export class ReplaceLabelReferencesPlugin implements CompilerPlugin {
  run(context: CompilationContext): CompilationContext {
    context.nodes.forEach((node) => {
      if (InstructionNode.isOfType(node, 'jumpTo') || InstructionNode.isOfType(node, 'jumpIf')) {
        if (node.address) {
          return;
        }

        const label = node.label.label;

        // if (!context.labelAddresses.has(label)) {
        //   throw new Error(`Label ${label} not found`);
        // }

        const address = context.labelAddresses.get(label);
        node.address = InstructionNode.create('numberValue', {
          dataType: ValueType.Address,
          value: address,
        }) as NumberValueNode;
      }
    });

    return context;
  }
}
