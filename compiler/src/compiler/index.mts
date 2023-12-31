import { Compiler, ByteArray } from './compiler.mjs';
import { defaultPlugins } from './plugins/index.mjs';
export * from './types/index.mjs';
export { Compiler, CompilerPlugin, ByteArray, CompilationContext } from './compiler.mjs';
export { CheckTypesPlugin, FindIdentifiersPlugin, FindLabelsPlugin, SerializePlugin } from './plugins/index.mjs';

export function compile(source: string): ByteArray {
  return new Compiler().compile(source || '', defaultPlugins);
}
