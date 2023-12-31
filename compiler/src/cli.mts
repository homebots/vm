import { createReadStream } from "node:fs";
import process from "node:process";
import { Buffer } from "node:buffer";
import { compile } from "./index.mjs";

function main() {
  try {
    const args = process.argv.slice(2);

    if (!args.length) {
      throw new Error("File input not specified");
    }

    const source = args[0] === "-" ? process.stdin : createReadStream(args[0]);
    const format = args[1] || undefined;
    const parts = [];
    source.on("data", (c) => parts.push(c));
    source.on("end", () => {
      const input = Buffer.concat(parts).toString("utf8");
      compileAndPrint(input, format);
    });
  } catch (error) {
    console.error(String(error));
    console.log(`
Usage: npx @homebots/espresso-compiler <file> <format>

<file>      path to .esp file, or "-" for stdin.
<format>    js, hex, utf8, base64. Defaults to binary

Examples:
  npx @homebots/espresso-compiler file.esp js
  cat file.esp | npx @homebots/espresso-compiler - hex
`);
  }
}

function compileAndPrint(source, format) {
  const buffer = Buffer.from(compile(source));

  if (format === "js") {
    console.log("export default [");
    buffer.forEach((byte) =>
      process.stdout.write("0x" + byte.toString(16) + ",\n")
    );
    console.log("];");
    return;
  }

  process.stdout.write(format ? buffer.toString(format) : buffer);
}

main();
