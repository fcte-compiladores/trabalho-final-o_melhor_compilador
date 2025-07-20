import { tokenize } from './tokenizer.js';

describe('tokenizer', () => {
  it('should handle empty input', () => {
    const input = '';
    const tokens = tokenize(input);
    expect(tokens).toEqual([{ type: 'EOF' }]);
  });

  it('should tokenize a simple command', () => {
    const input = 'FORWARD 100';
    const tokens = tokenize(input);
    expect(tokens).toEqual([
      { type: 'COMMAND', value: 'FORWARD' },
      { type: 'NUMBER', value: 100 },
      { type: 'EOF' },
    ]);
  });

  it('should tokenize a command with a color', () => {
    const input = 'BACKGROUND #FF0000';
    const tokens = tokenize(input);
    expect(tokens).toEqual([
      { type: 'COMMAND', value: 'BACKGROUND' },
      { type: 'IDENT', value: '#ff0000' },
      { type: 'EOF' },
    ]);
  });

  it('should tokenize a REPEAT block', () => {
    const input = 'REPEAT 4 [ FORWARD 100 RIGHT 90 ]';
    const tokens = tokenize(input);
    expect(tokens).toEqual([
      { type: 'COMMAND', value: 'REPEAT' },
      { type: 'NUMBER', value: 4 },
      { type: '[' },
      { type: 'COMMAND', value: 'FORWARD' },
      { type: 'NUMBER', value: 100 },
      { type: 'COMMAND', value: 'RIGHT' },
      { type: 'NUMBER', value: 90 },
      { type: ']' },
      { type: 'EOF' },
    ]);
  });

  it('should handle multiple commands', () => {
    const input = 'FD 50 RT 90';
    const tokens = tokenize(input);
    expect(tokens).toEqual([
      { type: 'COMMAND', value: 'FD' },
      { type: 'NUMBER', value: 50 },
      { type: 'COMMAND', value: 'RT' },
      { type: 'NUMBER', value: 90 },
      { type: 'EOF' },
    ]);
  });

  it('should handle commands with different cases', () => {
    const input = 'forward 100 rT 90';
    const tokens = tokenize(input);
    expect(tokens).toEqual([
      { type: 'COMMAND', value: 'FORWARD' },
      { type: 'NUMBER', value: 100 },
      { type: 'COMMAND', value: 'RT' },
      { type: 'NUMBER', value: 90 },
      { type: 'EOF' },
    ]);
  });

  it('should tokenize numbers with decimals', () => {
    const input = 'FORWARD 10.5';
    const tokens = tokenize(input);
    expect(tokens).toEqual([
      { type: 'COMMAND', value: 'FORWARD' },
      { type: 'NUMBER', value: 10.5 },
      { type: 'EOF' },
    ]);
  });

  it('should tokenize 3-digit hex colors', () => {
    const input = 'BG #F00';
    const tokens = tokenize(input);
    expect(tokens).toEqual([
      { type: 'COMMAND', value: 'BG' },
      { type: 'IDENT', value: '#f00' },
      { type: 'EOF' },
    ]);
  });

  it('should tokenize PENUP and PENDOWN', () => {
    const input = 'PENUP PENDOWN';
    const tokens = tokenize(input);
    expect(tokens).toEqual([
      { type: 'COMMAND', value: 'PENUP' },
      { type: 'COMMAND', value: 'PENDOWN' },
      { type: 'EOF' },
    ]);
  });

  it('should tokenize shape commands', () => {
    const input = 'CIRCLE 50 RECTANGLE 100 200 POLYGON 3 60';
    const tokens = tokenize(input);
    expect(tokens).toEqual([
      { type: 'COMMAND', value: 'CIRCLE' },
      { type: 'NUMBER', value: 50 },
      { type: 'COMMAND', value: 'RECTANGLE' },
      { type: 'NUMBER', value: 100 },
      { type: 'NUMBER', value: 200 },
      { type: 'COMMAND', value: 'POLYGON' },
      { type: 'NUMBER', value: 3 },
      { type: 'NUMBER', value: 60 },
      { type: 'EOF' },
    ]);
  });

  it('should tokenize CURVE command', () => {
    const input = 'CURVE 1 2 3 4 5 6';
    const tokens = tokenize(input);
    expect(tokens).toEqual([
      { type: 'COMMAND', value: 'CURVE' },
      { type: 'NUMBER', value: 1 },
      { type: 'NUMBER', value: 2 },
      { type: 'NUMBER', value: 3 },
      { type: 'NUMBER', value: 4 },
      { type: 'NUMBER', value: 5 },
      { type: 'NUMBER', value: 6 },
      { type: 'EOF' },
    ]);
  });
});