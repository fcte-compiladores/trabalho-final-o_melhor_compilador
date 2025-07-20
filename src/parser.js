import { tokenize } from "./tokenizer.js";

export function parse(input) {
  const tokens = tokenize(input);
  let pos = 0;
  const peek = () => tokens[pos];
  const consume = (t) => {
    const tk = tokens[pos];
    if (tk.type !== t && tk.value !== t)
      throw new Error(`Esperado ${t}, obtido ${tk.type}`);
    pos++;
    return tk;
  };

  function parseProgram() {
    const stmts = [];
    while (peek().type !== "EOF") stmts.push(parseStatement());
    return { type: "Program", body: stmts };
  }

  function parseStatement() {
    const tk = peek();
    if (tk.type === "COMMAND") {
      const cmd = consume("COMMAND").value;

      // REPEAT
      if (cmd === "REPEAT") {
        const count = consume("NUMBER").value;
        consume("[");
        const body = [];
        while (peek().type !== "]") body.push(parseStatement());
        consume("]");
        return { type: "Repeat", count, body };
      }

      // BACKGROUND
      if (cmd === "BACKGROUND" || cmd === "BG") {
        const color = consume("IDENT").value;
        return { type: "Background", color };
      }

      // PEN
      if (cmd === "PENUP" || cmd === "PU") return { type: "Pen", state: "up" };
      if (cmd === "PENDOWN" || cmd === "PD")
        return { type: "Pen", state: "down" };

      // MOVE
      if (/FORWARD|FD|BACK|BK/.test(cmd)) {
        const num = consume("NUMBER").value;
        const dist = /BACK|BK/.test(cmd) ? -num : num;
        let color = "black";
        if (peek().type === "IDENT") color = consume("IDENT").value;
        return { type: "Move", distance: dist, color };
      }

      // TURN
      if (/RIGHT|RT|LEFT|LT/.test(cmd)) {
        const num = consume("NUMBER").value;
        const angle = /LEFT|LT/.test(cmd) ? -num : num;
        let color = null;
        if (peek().type === "IDENT") color = consume("IDENT").value;
        return { type: "Turn", angle, color };
      }

      // SHAPES
      if (cmd === "CIRCLE") {
        const r = consume("NUMBER").value;
        let color = "black";
        if (peek().type === "IDENT") color = consume("IDENT").value;
        return { type: "Circle", radius: r, color };
      }
      if (cmd === "RECTANGLE") {
        const w = consume("NUMBER").value;
        const h = consume("NUMBER").value;
        let color = "black";
        if (peek().type === "IDENT") color = consume("IDENT").value;
        return { type: "Rectangle", width: w, height: h, color };
      }
      if (cmd === "POLYGON") {
        const n = consume("NUMBER").value;
        const len = consume("NUMBER").value;
        let color = "black";
        if (peek().type === "IDENT") color = consume("IDENT").value;
        return { type: "Polygon", sides: n, size: len, color };
      }

      // CURVE
      if (cmd === "CURVE") {
        const pts = [];
        for (let i = 0; i < 6; i++) pts.push(consume("NUMBER").value);
        let color = "black";
        if (peek().type === "IDENT") color = consume("IDENT").value;
        return { type: "Curve", pts, color };
      }

      throw new Error(`Comando desconhecido: ${cmd}`);
    }
    throw new Error(`Token inesperado: ${tk.type}`);
  }

  return parseProgram();
}
