export function interpret(ast) {
  const turtle = { x: 0, y: 0, angle: 0, penDown: true };
  const segments = [];
  const shapes = [];
  const curves = [];
  let backgroundColor = null;

  function visit(node) {
    switch (node.type) {
      case "Program":
        return node.body.forEach(visit);

      case "Repeat":
        for (let i = 0; i < node.count; i++) node.body.forEach(visit);
        return;

      case "Background":
        backgroundColor = node.color;
        return;

      case "Pen":
        turtle.penDown = node.state === "down";
        return;

      case "Move": {
        const rad = (turtle.angle * Math.PI) / 180;
        const nx = turtle.x + Math.cos(rad) * node.distance;
        const ny = turtle.y + Math.sin(rad) * node.distance;
        if (turtle.penDown) {
          segments.push({
            x1: turtle.x,
            y1: turtle.y,
            x2: nx,
            y2: ny,
            color: node.color,
          });
        }
        turtle.x = nx;
        turtle.y = ny;
        return;
      }

      case "Turn":
        if (node.color) {
          const ny = turtle.y,
            nx = turtle.x;
          segments.push({ x1: nx, y1: ny, x2: nx, y2: ny, color: node.color });
        }
        turtle.angle = (turtle.angle + node.angle) % 360;
        return;

      case "Circle":
        shapes.push({
          type: "Circle",
          cx: turtle.x,
          cy: turtle.y,
          r: node.radius,
          color: node.color,
        });
        return;

      case "Rectangle":
        shapes.push({
          type: "Rectangle",
          x: turtle.x,
          y: turtle.y,
          width: node.width,
          height: node.height,
          color: node.color,
        });
        return;

      case "Polygon":
        shapes.push({
          type: "Polygon",
          cx: turtle.x,
          cy: turtle.y,
          sides: node.sides,
          size: node.size,
          color: node.color,
        });
        return;

      case "Curve":
        curves.push({
          type: "Curve",
          x0: turtle.x,
          y0: turtle.y,
          pts: node.pts,
          color: node.color,
        });
        return;

      default:
        throw new Error(`Nó desconhecido: ${node.type}`);
    }
  }

  visit(ast);
  return { segments, shapes, curves, backgroundColor };
}
