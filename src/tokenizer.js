const COMMANDS = new Set([
  "FORWARD",
  "FD",
  "BACK",
  "BK",
  "RIGHT",
  "RT",
  "LEFT",
  "LT",
  "PENUP",
  "PU",
  "PENDOWN",
  "PD",
  "BACKGROUND",
  "BG",
  "REPEAT",
  "CIRCLE",
  "RECTANGLE",
  "POLYGON",
  "CURVE",
]);

export function tokenize(input) {
  const tokens = [];
  const re = /\s*([A-Za-z]+|\d+(?:\.\d+)?|#[0-9A-Fa-f]{3,6}|\[|\]|,)\s*/g;
  let match;
  while ((match = re.exec(input)) !== null) {
    const txt = match[1];
    if (!isNaN(txt)) {
      tokens.push({ type: "NUMBER", value: parseFloat(txt) });
    } else if (txt === "[" || txt === "]") {
      tokens.push({ type: txt });
    } else if (txt.startsWith("#")) {
      tokens.push({ type: "IDENT", value: txt.toLowerCase() });
    } else {
      const up = txt.toUpperCase();
      if (COMMANDS.has(up)) tokens.push({ type: "COMMAND", value: up });
      else tokens.push({ type: "IDENT", value: txt.toLowerCase() });
    }
  }
  tokens.push({ type: "EOF" });
  return tokens;
}
