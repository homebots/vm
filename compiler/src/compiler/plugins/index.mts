import { CompilerPlugin } from '../compiler.mjs';
import { CheckTypesPlugin } from './check-types.plugin.mjs';
import { FindIdentifiersPlugin, ReplaceIdentifiersPlugin } from './identifiers.plugin.mjs';
import { FindLabelsPlugin, ReplaceLabelReferencesPlugin } from './references.plugin.mjs';
import { SerializePlugin } from './serialize.plugin.mjs';

export const defaultPlugins: CompilerPlugin[] = [
  new FindIdentifiersPlugin(),
  new FindLabelsPlugin(),
  new CheckTypesPlugin(),
  new ReplaceLabelReferencesPlugin(),
  new ReplaceIdentifiersPlugin(),
  new SerializePlugin(),
];

export { CheckTypesPlugin, FindIdentifiersPlugin, FindLabelsPlugin, SerializePlugin };
