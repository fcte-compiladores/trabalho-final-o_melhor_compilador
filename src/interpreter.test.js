import { interpret } from './interpreter.js';
import { parse } from './parser.js';

describe('interpreter', () => {
  it('should interpret a simple FORWARD command', () => {
    const input = 'FORWARD 100';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.segments.length).toBe(1);
    expect(result.segments[0].x1).toBe(0);
    expect(result.segments[0].y1).toBe(0);
    expect(result.segments[0].x2).toBe(100);
    expect(result.segments[0].y2).toBe(0);
  });

  it('should interpret a BACK command', () => {
    const input = 'BACK 50';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.segments.length).toBe(1);
    expect(result.segments[0].x1).toBe(0);
    expect(result.segments[0].y1).toBe(0);
    expect(result.segments[0].x2).toBe(-50);
    expect(result.segments[0].y2).toBe(0);
  });

  it('should interpret a REPEAT block', () => {
    const input = 'REPEAT 4 [ FORWARD 10 RIGHT 90 ]';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.segments.length).toBe(4);
    expect(result.segments[3].x2).toBeCloseTo(0);
    expect(result.segments[3].y2).toBeCloseTo(0);
  });

  it('should interpret a BACKGROUND command', () => {
    const input = 'BACKGROUND #FF0000';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.backgroundColor).toBe('#ff0000');
  });

  it('should interpret PENUP and PENDOWN commands', () => {
    const input = 'FORWARD 10 PENUP FORWARD 10 PENDOWN FORWARD 10';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.segments.length).toBe(2);
  });

  it('should interpret shape commands', () => {
    const input = 'CIRCLE 50 RECTANGLE 100 200 POLYGON 6 50';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.shapes.length).toBe(3);
    expect(result.shapes[0].type).toBe('Circle');
    expect(result.shapes[1].type).toBe('Rectangle');
    expect(result.shapes[2].type).toBe('Polygon');
  });

  it('should interpret a CURVE command', () => {
    const input = 'CURVE 10 20 30 40 50 60';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.curves.length).toBe(1);
    expect(result.curves[0].pts).toEqual([10, 20, 30, 40, 50, 60]);
  });

  it('should interpret LEFT and RIGHT commands', () => {
    const input = 'RIGHT 90 LEFT 45';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.segments.length).toBe(0);
    // The turtle's angle should be 90 - 45 = 45 degrees
    // We can't directly test the turtle's angle, but we can test its effect on subsequent moves.
    const input2 = 'RIGHT 90 FORWARD 100';
    const ast2 = parse(input2);
    const result2 = interpret(ast2);
    expect(result2.segments[0].x2).toBeCloseTo(0);
    expect(result2.segments[0].y2).toBeCloseTo(100);
  });

  it('should interpret Move command with color', () => {
    const input = 'FORWARD 100 #FF0000';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.segments[0].color).toBe('#ff0000');
  });

  it('should interpret Turn command with color', () => {
    const input = 'RIGHT 90 #00FF00';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.segments[0].color).toBe('#00ff00');
  });

  it('should interpret nested REPEAT blocks', () => {
    const input = 'REPEAT 2 [ FORWARD 10 REPEAT 2 [ RIGHT 90 ] ]';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.segments.length).toBe(2); // Two FORWARD commands
  });

  it('should interpret Circle command with color', () => {
    const input = 'CIRCLE 50 #123456';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.shapes[0].color).toBe('#123456');
  });

  it('should interpret Rectangle command with color', () => {
    const input = 'RECTANGLE 100 200 #7890AB';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.shapes[0].color).toBe('#7890ab');
  });

  it('should interpret Polygon command with color', () => {
    const input = 'POLYGON 3 60 #CDEF01';
    const ast = parse(input);
    const result = interpret(ast);
    expect(result.shapes[0].color).toBe('#cdef01');
  });

  it('should throw an error for unknown node type', () => {
    const ast = { type: 'Program', body: [{ type: 'Unknown' }] };
    expect(() => interpret(ast)).toThrow('Nó desconhecido: Unknown');
  });
});