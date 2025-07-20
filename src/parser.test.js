import { parse } from './parser.js';

describe('parser', () => {
  it('should parse a simple command', () => {
    const input = 'FORWARD 100';
    const ast = parse(input);
    expect(ast).toEqual({
      type: 'Program',
      body: [
        {
          type: 'Move',
          distance: 100,
          color: 'black',
        },
      ],
    });
  });

  it('should parse a REPEAT block', () => {
    const input = 'REPEAT 4 [ FORWARD 100 RIGHT 90 ]';
    const ast = parse(input);
    expect(ast).toEqual({
      type: 'Program',
      body: [
        {
          type: 'Repeat',
          count: 4,
          body: [
            {
              type: 'Move',
              distance: 100,
              color: 'black',
            },
            {
              type: 'Turn',
              angle: 90,
              color: null,
            },
          ],
        },
      ],
    });
  });

  it('should parse a BACKGROUND command', () => {
    const input = 'BACKGROUND #FF0000';
    const ast = parse(input);
    expect(ast).toEqual({
      type: 'Program',
      body: [
        {
          type: 'Background',
          color: '#ff0000',
        },
      ],
    });
  });

  it('should parse PENUP and PENDOWN commands', () => {
    const input = 'PENUP PENDOWN';
    const ast = parse(input);
    expect(ast).toEqual({
      type: 'Program',
      body: [
        {
          type: 'Pen',
          state: 'up',
        },
        {
          type: 'Pen',
          state: 'down',
        },
      ],
    });
  });

  it('should parse shape commands', () => {
    const input = 'CIRCLE 50 RECTANGLE 100 200 POLYGON 6 50';
    const ast = parse(input);
    expect(ast).toEqual({
      type: 'Program',
      body: [
        {
          type: 'Circle',
          radius: 50,
          color: 'black',
        },
        {
          type: 'Rectangle',
          width: 100,
          height: 200,
          color: 'black',
        },
        {
          type: 'Polygon',
          sides: 6,
          size: 50,
          color: 'black',
        },
      ],
    });
  });

  it('should throw an error for unknown commands', () => {
    const input = 'JUMP 100';
    expect(() => parse(input)).toThrow('Token inesperado: IDENT');
  });

  it('should throw an error for unexpected tokens', () => {
    const input = 'FORWARD [ 100 ]';
    expect(() => parse(input)).toThrow('Esperado NUMBER, obtido [');
  });

  it('should parse nested REPEAT blocks', () => {
    const input = 'REPEAT 2 [ FORWARD 10 REPEAT 2 [ RIGHT 90 ] ]';
    const ast = parse(input);
    expect(ast).toEqual({
      type: 'Program',
      body: [
        {
          type: 'Repeat',
          count: 2,
          body: [
            {
              type: 'Move',
              distance: 10,
              color: 'black',
            },
            {
              type: 'Repeat',
              count: 2,
              body: [
                {
                  type: 'Turn',
                  angle: 90,
                  color: null,
                },
              ],
            },
          ],
        },
      ],
    });
  });

  it('should parse commands with optional color arguments', () => {
    const input = 'FORWARD 100 #00FF00 RIGHT 90 #0000FF';
    const ast = parse(input);
    expect(ast).toEqual({
      type: 'Program',
      body: [
        {
          type: 'Move',
          distance: 100,
          color: '#00ff00',
        },
        {
          type: 'Turn',
          angle: 90,
          color: '#0000ff',
        },
      ],
    });
  });

  it('should throw an error for unmatched opening bracket', () => {
    const input = 'REPEAT 2 [ FORWARD 10';
    expect(() => parse(input)).toThrow('Token inesperado: EOF');
  });

  it('should throw an error for unmatched closing bracket', () => {
    const input = 'FORWARD 10 ]';
    expect(() => parse(input)).toThrow('Token inesperado: ]');
  });

  it('should parse CURVE command with color', () => {
    const input = 'CURVE 1 2 3 4 5 6 #ABCDEF';
    const ast = parse(input);
    expect(ast).toEqual({
      type: 'Program',
      body: [
        {
          type: 'Curve',
          pts: [1, 2, 3, 4, 5, 6],
          color: '#abcdef',
        },
      ],
    });
  });

  it('should parse CIRCLE command with color', () => {
    const input = 'CIRCLE 50 #123456';
    const ast = parse(input);
    expect(ast).toEqual({
      type: 'Program',
      body: [
        {
          type: 'Circle',
          radius: 50,
          color: '#123456',
        },
      ],
    });
  });

  it('should parse RECTANGLE command with color', () => {
    const input = 'RECTANGLE 100 200 #7890AB';
    const ast = parse(input);
    expect(ast).toEqual({
      type: 'Program',
      body: [
        {
          type: 'Rectangle',
          width: 100,
          height: 200,
          color: '#7890ab',
        },
      ],
    });
  });

  it('should parse POLYGON command with color', () => {
    const input = 'POLYGON 3 60 #CDEF01';
    const ast = parse(input);
    expect(ast).toEqual({
      type: 'Program',
      body: [
        {
          type: 'Polygon',
          sides: 3,
          size: 60,
          color: '#cdef01',
        },
      ],
    });
  });
});