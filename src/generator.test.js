import { generateSVG } from './generator.js';
import fs from 'fs';
import { jest } from '@jest/globals';

describe('generator', () => {
  let writeFileSyncSpy;

  beforeEach(() => {
    writeFileSyncSpy = jest.spyOn(fs, 'writeFileSync').mockImplementation(() => {});
  });

  afterEach(() => {
    writeFileSyncSpy.mockRestore();
  });

  it('should generate an SVG with a single line segment', () => {
    const data = {
      segments: [{ x1: 0, y1: 0, x2: 100, y2: 0, color: 'black' }],
      shapes: [],
      curves: [],
      backgroundColor: null,
    };
    generateSVG(data, 'test.svg');

    expect(fs.writeFileSync).toHaveBeenCalledTimes(1);
    const svgContent = writeFileSyncSpy.mock.calls[0][1];
    expect(svgContent).toContain('<line x1="10" y1="10" x2="110" y2="10" stroke="black" stroke-width="2"/>');
  });

  it('should generate an SVG with a background color', () => {
    const data = {
      segments: [{ x1: 0, y1: 0, x2: 0, y2: 0, color: 'transparent' }],
      shapes: [],
      curves: [],
      backgroundColor: '#ff0000',
    };
    generateSVG(data, 'test.svg');

    expect(fs.writeFileSync).toHaveBeenCalledTimes(1);
    const svgContent = writeFileSyncSpy.mock.calls[0][1];
    expect(svgContent).toContain('<rect width="20" height="20" fill="#ff0000"/>');
  });

  it('should generate an SVG with shapes', () => {
    const data = {
      segments: [],
      shapes: [
        { type: 'Circle', cx: 50, cy: 50, r: 20, color: 'blue' },
        { type: 'Rectangle', x: 100, y: 100, width: 50, height: 30, color: 'green' },
        { type: 'Polygon', cx: 150, cy: 150, sides: 6, size: 25, color: 'purple' },
      ],
      curves: [],
      backgroundColor: null,
    };
    generateSVG(data, 'test.svg');

    expect(fs.writeFileSync).toHaveBeenCalledTimes(1);
    const svgContent = writeFileSyncSpy.mock.calls[0][1];
    expect(svgContent).toContain('<circle');
    expect(svgContent).toContain('<rect');
    expect(svgContent).toContain('polygon');
  });

  it('should generate an SVG with curves', () => {
    const data = {
      segments: [],
      shapes: [],
      curves: [
        { type: 'Curve', x0: 0, y0: 0, pts: [10, 20, 30, 40, 50, 60], color: 'red' },
      ],
      backgroundColor: null,
    };
    generateSVG(data, 'test.svg');

    expect(fs.writeFileSync).toHaveBeenCalledTimes(1);
    const svgContent = writeFileSyncSpy.mock.calls[0][1];
    expect(svgContent).toContain('<path d="M10,70 C20,50 40,30 60,10" fill="none" stroke="red" stroke-width="2"/>');
  });

  it('should generate an SVG with multiple line segments', () => {
    const data = {
      segments: [
        { x1: 0, y1: 0, x2: 10, y2: 0, color: 'black' },
        { x1: 10, y1: 0, x2: 10, y2: 10, color: 'blue' },
      ],
      shapes: [],
      curves: [],
      backgroundColor: null,
    };
    generateSVG(data, 'test.svg');

    expect(fs.writeFileSync).toHaveBeenCalledTimes(1);
    const svgContent = writeFileSyncSpy.mock.calls[0][1];
    expect(svgContent).toContain('<line x1="10" y1="20" x2="20" y2="20" stroke="black" stroke-width="2"/>');
    expect(svgContent).toContain('<line x1="20" y1="20" x2="20" y2="10" stroke="blue" stroke-width="2"/>');
  });

  it('should generate an SVG with a combination of segments, shapes, and curves', () => {
    const data = {
      segments: [{ x1: 0, y1: 0, x2: 10, y2: 0, color: 'black' }],
      shapes: [
        { type: 'Circle', cx: 50, cy: 50, r: 20, color: 'blue' },
      ],
      curves: [
        { type: 'Curve', x0: 0, y0: 0, pts: [10, 20, 30, 40, 50, 60], color: 'red' },
      ],
      backgroundColor: '#cccccc',
    };
    generateSVG(data, 'test.svg');

    expect(fs.writeFileSync).toHaveBeenCalledTimes(1);
    const svgContent = writeFileSyncSpy.mock.calls[0][1];
    expect(svgContent).toContain('<rect');
    expect(svgContent).toContain('<line');
    expect(svgContent).toContain('<circle');
    expect(svgContent).toContain('<path');
  });

  it('should generate an SVG when given an array of segments directly', () => {
    const segments = [
      { x1: 0, y1: 0, x2: 100, y2: 0, color: 'black' },
    ];
    generateSVG(segments, 'test.svg', '#abcdef');

    expect(fs.writeFileSync).toHaveBeenCalledTimes(1);
    const svgContent = writeFileSyncSpy.mock.calls[0][1];
    expect(svgContent).toContain('<line x1="10" y1="10" x2="110" y2="10" stroke="black" stroke-width="2"/>');
    expect(svgContent).toContain('<rect width="120" height="20" fill="#abcdef"/>');
  });

  it('should generate a valid SVG even with no segments, shapes, or curves', () => {
    const data = {
      segments: [],
      shapes: [],
      curves: [],
      backgroundColor: null,
    };
    generateSVG(data, 'test.svg');

    expect(fs.writeFileSync).toHaveBeenCalledTimes(1);
    const svgContent = writeFileSyncSpy.mock.calls[0][1];
    expect(svgContent).toContain('<svg');
    expect(svgContent).toContain('width="20" height="20"'); // Default dimensions
  });
});