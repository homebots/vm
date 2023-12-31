module.exports = {
  root: true,
  parser: '@typescript-eslint/parser',
  plugins: ['@typescript-eslint'],
  extends: ['eslint:recommended', 'plugin:@typescript-eslint/recommended'],
  rules: {
    'one-var': [2, 'never'],
    'no-control-regex': [0],
    'prefer-const': [2],
    '@typescript-eslint/no-unused-vars': [2, { args: 'none' }],
  },
};
