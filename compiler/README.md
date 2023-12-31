# Espresso compiler

Compiler for Espresso language, used by [HomeBots VM](https://github.com/homebots/vm)

## Usage

```
npx @homebots/espresso-compiler <input> <output-format>
```

`<input>`: A script file, or `-` to read from stdin

`<output-format>`: One of the Node.js Buffer string formats (hex, utf8, base64...), or specify `js` to output a valid JS module with an array of bytes with the program

```sh
npx @homebots/espresso-compiler path/to/file.esp hex
cat file.esp | npx @homebots/espresso-compiler - js
```