const HEIGHT = 20;
const WIDTH = 15;
const SIZE = 4;

const TEMPLATES = {
  I: ["....", "I...", "I...", "I..."],
  O: [".OO.", ".OO.", "....", "...."],
  T: [".T..", "TTT.", "....", "...."],
  S: [".SS.", "SS..", "....", "...."],
  Z: ["ZZ..", ".ZZ.", "....", "...."],
  J: ["J...", "JJJ.", "....", "...."],
  L: ["..L.", "LLL.", "....", "...."],
};

function gridFor(type) {
  return TEMPLATES[type].map((row) => [...row].map((cell) => cell === "." ? " " : cell));
}

function rotate(grid) {
  return Array.from({ length: SIZE }, (_, row) =>
    Array.from({ length: SIZE }, (_, column) => grid[SIZE - 1 - column][row]));
}

export class Piece {
  constructor(type, cells = gridFor(type)) {
    this.type = type;
    this.cells = cells;
  }

  rotated() {
    return new Piece(this.type, rotate(this.cells));
  }

  static random() {
    const types = Object.keys(TEMPLATES);
    return new Piece(types[Math.floor(Math.random() * types.length)]);
  }
}

export class Board {
  constructor() {
    this.reset();
  }

  reset() {
    this.cells = Array.from({ length: HEIGHT }, (_, row) =>
      Array.from({ length: WIDTH }, (_, column) =>
        row === 0 || row === HEIGHT - 1 || column === 0 || column === WIDTH - 1 ? "#" : " "));
  }

  canPlace(piece, x, y) {
    for (let row = 0; row < SIZE; row += 1) {
      for (let column = 0; column < SIZE; column += 1) {
        if (piece.cells[row][column] === " ") continue;
        const boardX = x + column;
        const boardY = y + row;
        if (boardX < 1 || boardX >= WIDTH - 1 || boardY < 1 ||
            boardY >= HEIGHT - 1 || this.cells[boardY][boardX] !== " ") return false;
      }
    }
    return true;
  }

  lock(piece, x, y) {
    piece.cells.forEach((row, rowIndex) => row.forEach((cell, columnIndex) => {
      if (cell !== " ") this.cells[y + rowIndex][x + columnIndex] = cell;
    }));
  }

  clearCompletedLines() {
    const cleared = [];
    for (let row = HEIGHT - 2; row > 0; row -= 1) {
      if (this.cells[row].slice(1, WIDTH - 1).every((cell) => cell !== " ")) {
        cleared.push(row);
        for (let copyRow = row; copyRow > 1; copyRow -= 1) this.cells[copyRow] = [...this.cells[copyRow - 1]];
        this.cells[1] = Array(WIDTH).fill(" ");
        this.cells[1][0] = "#";
        this.cells[1][WIDTH - 1] = "#";
        row += 1;
      }
    }
    return cleared;
  }

  dropDistance(piece, x, y) {
    let distance = 0;
    while (this.canPlace(piece, x, y + distance + 1)) distance += 1;
    return distance;
  }
}

export class Score {
  constructor() { this.value = 0; this.lines = 0; }
  addLines(count) {
    const points = [0, 100, 300, 500, 800];
    if (count >= 1 && count <= 4) {
      this.value += points[count];
      this.lines += count;
    }
  }
  get level() { return Math.floor(this.lines / 10) + 1; }
}

export class Game {
  constructor() { this.restart(); }

  restart() {
    this.board = new Board();
    this.score = new Score();
    this.active = Piece.random();
    this.next = Piece.random();
    this.x = 5;
    this.y = 1;
    this.paused = false;
    this.over = false;
    this.lastFall = performance.now();
  }

  move(dx, dy) {
    if (this.board.canPlace(this.active, this.x + dx, this.y + dy)) {
      this.x += dx; this.y += dy; return true;
    }
    return false;
  }

  rotate() {
    const rotated = this.active.rotated();
    const kicks = [[0, 0], [-1, 0], [1, 0], [-2, 0], [2, 0], [0, -1], [0, 1], [-1, -1], [1, -1]];
    for (const [dx, dy] of kicks) {
      if (this.board.canPlace(rotated, this.x + dx, this.y + dy)) {
        this.active = rotated; this.x += dx; this.y += dy; return true;
      }
    }
    return false;
  }

  hardDrop() {
    this.y += this.board.dropDistance(this.active, this.x, this.y);
    return this.lock();
  }

  tick(now) {
    if (this.paused || this.over) return null;
    const interval = Math.max(80, 500 - (this.score.level - 1) * 40);
    if (now - this.lastFall < interval) return null;
    this.lastFall = now;
    if (this.move(0, 1)) return null;
    return this.lock();
  }

  lock() {
    this.board.lock(this.active, this.x, this.y);
    const clearedRows = this.board.clearCompletedLines();
    this.score.addLines(clearedRows.length);
    this.active = this.next;
    this.next = Piece.random();
    this.x = 5; this.y = 1;
    if (!this.board.canPlace(this.active, this.x, this.y)) this.over = true;
    return clearedRows;
  }
}

export { HEIGHT, WIDTH, SIZE };
