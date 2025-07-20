import fs from "fs";

export function generateSVG(dataOrSegments, outFile, backgroundColorArg) {
  let segments, shapes, curves, bgColor;
  if (Array.isArray(dataOrSegments)) {
    segments = dataOrSegments;
    shapes = [];
    curves = [];
    bgColor = backgroundColorArg;
  } else {
    segments = dataOrSegments.segments;
    shapes = dataOrSegments.shapes || [];
    curves = dataOrSegments.curves || [];
    bgColor =
      dataOrSegments.backgroundColor != null
        ? dataOrSegments.backgroundColor
        : backgroundColorArg;
  }

  const xs = segments.flatMap((s) => [s.x1, s.x2]);
  const ys = segments.flatMap((s) => [s.y1, s.y2]);
  shapes.forEach((sh) => {
    if (sh.type === "Circle") {
      xs.push(sh.cx - sh.r, sh.cx + sh.r);
      ys.push(sh.cy - sh.r, sh.cy + sh.r);
    }
    if (sh.type === "Rectangle") {
      xs.push(sh.x, sh.x + sh.width);
      ys.push(sh.y, sh.y + sh.height);
    }
    if (sh.type === "Polygon") {
      xs.push(sh.cx - sh.size, sh.cx + sh.size);
      ys.push(sh.cy - sh.size, sh.cy + sh.size);
    }
  });
  curves.forEach((cv) => {
    const [x1, y1, x2, y2, x3, y3] = cv.pts;
    xs.push(cv.x0, cv.x0 + x1, cv.x0 + x2, cv.x0 + x3);
    ys.push(cv.y0, cv.y0 + y1, cv.y0 + y2, cv.y0 + y3);
  });

  // Ensure there's at least one point to calculate bounds
  if (xs.length === 0) xs.push(0);
  if (ys.length === 0) ys.push(0);

  const minX = Math.min(...xs),
    maxX = Math.max(...xs);
  const minY = Math.min(...ys),
    maxY = Math.max(...ys);
  const width = maxX - minX + 20;
  const height = maxY - minY + 20;
  const offX = -minX + 10;
  const offY = -minY + 10;

  const bg = bgColor
    ? `<rect width="${width}" height="${height}" fill="${bgColor}"/>\n`
    : "";

  const shapeEls = shapes
    .map((sh) => {
      if (sh.type === "Circle") {
        return `<circle cx="${sh.cx + offX}" cy="${
          height - (sh.cy + offY)
        }" r="${sh.r}" fill="none" stroke="${sh.color}" stroke-width="2"/>`;
      }
      if (sh.type === "Rectangle") {
        return `<rect x="${sh.x + offX}" y="${
          height - (sh.y + offY) - sh.height
        }" width="${sh.width}" height="${sh.height}" fill="none" stroke="${
          sh.color
        }" stroke-width="2"/>`;
      }
      if (sh.type === "Polygon") {
        const pts = [];
        for (let i = 0; i < sh.sides; i++) {
          const ang = (2 * Math.PI * i) / sh.sides + Math.PI / 2;
          const x = sh.cx + Math.cos(ang) * sh.size;
          const y = sh.cy + Math.sin(ang) * sh.size;
          pts.push(`${x + offX},${height - (y + offY)}`);
        }
        return `<polygon points="${pts.join(" ")}" fill="none" stroke="${
          sh.color
        }" stroke-width="2"/>`;
      }
    })
    .join("\n");

  const curveEls = curves
    .map((cv) => {
      console.log(cv.pts);
      const [x1, y1, x2, y2, x3, y3] = cv.pts;
      const x3p = cv.x0 + x3 + offX,
        y3p = height - (cv.y0 + y3 + offY);
      return `<path d="M${x1},${y1} C${x2},${y2} ${x3},${y3} ${x3p},${y3p}" fill="none" stroke="${cv.color}" stroke-width="2"/>`;
    })
    .join("\n");

  const segEls = segments
    .map((s) => {
      return `<line x1="${s.x1 + offX}" y1="${height - (s.y1 + offY)}" x2="${
        s.x2 + offX
      }" y2="${height - (s.y2 + offY)}" stroke="${s.color}" stroke-width="2"/>`;
    })
    .join("\n");

  const svg = `<svg xmlns="http://www.w3.org/2000/svg" width="${width}" height="${height}" viewBox="0 0 ${width} ${height}">\n${bg}${shapeEls}\n${curveEls}\n${segEls}\n</svg>`;

  fs.writeFileSync(outFile, svg, "utf-8");
}
