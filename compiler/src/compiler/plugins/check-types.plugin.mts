import { CompilationContext, CompilerPlugin } from '../compiler.mjs';
import { InstructionNode, ValueType } from '../types/index.mjs';

export class CheckTypesPlugin implements CompilerPlugin {
  run(context: CompilationContext): CompilationContext {
    context.nodes.forEach((node) => {
      if (InstructionNode.isOfType(node, 'declareIdentifier') && node.dataType !== node.value.dataType) {
        throw new Error(
          `Invalid value. Expected ${ValueType[node.dataType]} but found ${ValueType[node.value.dataType]}`,
        );
      }

      if (InstructionNode.isOfType(node, 'assign')) {
        const targetName = node.target.value.name;
        const targetType = context.identifierTypes.get(targetName);
        let assignedType = node.value.dataType;

        if (InstructionNode.isOfType(node.value, 'identifierValue')) {
          assignedType = context.identifierTypes.get(node.value.value.name);
        }

        if (targetType !== assignedType) {
          throw new Error(
            `Invalid value for ${targetName}. Expected ${ValueType[targetType]} but found ${ValueType[assignedType]}`,
          );
        }
      }
    });

    return context;
  }
}
