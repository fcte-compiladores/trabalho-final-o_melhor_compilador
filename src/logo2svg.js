import fs from "fs";
import { parse } from "./parser.js";
import { interpret } from "./interpreter.js";
import { generateSVG } from "./generator.js";

const [, , inputFile] = process.argv;
if (!inputFile) {
  console.error("Uso: node logo2svg.js <arquivo.logo> [<saida.svg>]");
  process.exit(1);
}
const filename = inputFile.replace(/\\/g, "/").split("/");
const baseName = filename[filename.length - 1].replace(/\.[^.]+$/, "") + ".svg";
const source = fs.readFileSync(inputFile, "utf-8");
const ast = parse(source);
console.log("=== AST ===\n", JSON.stringify(ast, null, 2));
const data = interpret(ast);
console.log("=== Segments ===\n", data.segments);
console.log("=== Shapes ===\n", data.shapes);
console.log("=== Curves ===\n", data.curves);
console.log("=== Background ===\n", data.backgroundColor);
generateSVG(data, baseName, null);
console.log(`SVG gerado em ${baseName}`);
