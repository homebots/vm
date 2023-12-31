import baseConfig from '@cloud-cli/jest-config';

/** @type {import('jest').Config} */
const config = {
  ...baseConfig,
  coveragePathIgnorePatterns: [
    '/node_modules/',
    '/__tests__/',
    '/.spec.[jt]s$/',
    'src/grammar.mts',
    'src/compiler/parser.mts',
    'src/index.mts',
    'src/compiler/index.mts',
    'src/compiler/plugins/index.mts',
    'src/compiler/types/index.mts',
    'src/emulator/index.mts',
  ],
  transformIgnorePatterns: ['/node_modules/', '/\\.mjs$/', '/\\.js$/'],
  testPathIgnorePatterns: ['/node_modules/'],
  testMatch: ['**/*.spec.[tj]s', '**/*.spec.m[tj]s'],
  moduleFileExtensions: ['ts', 'mts', 'js', 'mjs', 'cjs', 'json'],
  extensionsToTreatAsEsm: ['.ts', '.mts'],
  moduleNameMapper: {
    '^(\\.{1,2}/.*)\\.[m]?js$': '$1',
  },
  transform: {
    '^.+\\.[tj]s?$': ['ts-jest', { useESM: true }],
    '^.+\\.m[tj]s$': ['ts-jest', { useESM: true }],
  },
};

export default config;
