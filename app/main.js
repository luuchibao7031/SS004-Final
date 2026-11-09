import { Game, HEIGHT, WIDTH, SIZE } from "./game.js";
import { isSoundEnabled, playSound, setSoundEnabled } from "./sound.js";

const game = new Game();
const boardElement = document.querySelector("#board");
const nextElement = document.querySelector("#next");
const effectsElement = document.querySelector("#effects");
const overlay = document.querySelector("#overlay");
const overlayTitle = document.querySelector("#overlayTitle");
const overlayEyebrow = document.querySelector("#overlayEyebrow");
const overlayButton = document.querySelector("#overlayButton");
const highScoreElement = document.querySelector("#highScore");
let highScore = Number(localStorage.getItem("block-bloom-high-score") || 0);

const cells = Array.from({ length: HEIGHT * WIDTH }, (_, index) => {
  const cell = document.createElement("div");
  cell.className = "cell";
  cell.setAttribute("role", "gridcell");
  boardElement.append(cell);
  return cell;
});

function drawPiece(target, piece) {
  target.replaceChildren();
  for (let row = 0; row < SIZE; row += 1) {
    for (let column = 0; column < SIZE; column += 1) {
      const cell = document.createElement("div");
      const type = piece.cells[row][column];
      cell.className = `mini-cell ${type === " " ? "" : `piece-${type}`}`;
      target.append(cell);
    }
  }
}

function draw() {
  const visible = game.board.cells.map((row) => [...row]);
  const ghostY = game.y + game.board.dropDistance(game.active, game.x, game.y);
  game.active.cells.forEach((row, rowIndex) => row.forEach((type, columnIndex) => {
    if (type !== " ") visible[ghostY + rowIndex][game.x + columnIndex] = `ghost-${type}`;
  }));
  game.active.cells.forEach((row, rowIndex) => row.forEach((type, columnIndex) => {
    if (type !== " ") visible[game.y + rowIndex][game.x + columnIndex] = type;
  }));
  cells.forEach((cell, index) => {
    const value = visible[Math.floor(index / WIDTH)][index % WIDTH];
    cell.className = `cell ${value === "#" ? "wall" : value === " " ? "" : value.startsWith("ghost-") ? value : `piece-${value}`}`;
  });
  drawPiece(nextElement, game.next);
  document.querySelector("#score").textContent = game.score.value;
  document.querySelector("#lines").textContent = game.score.lines;
  document.querySelector("#level").textContent = game.score.level;
  highScore = Math.max(highScore, game.score.value);
  highScoreElement.textContent = highScore;
  localStorage.setItem("block-bloom-high-score", String(highScore));
}

function showOverlay(title, eyebrow, buttonText) {
  overlayTitle.textContent = title;
  overlayEyebrow.textContent = eyebrow;
  overlayButton.textContent = buttonText;
  overlay.classList.remove("hidden");
}

function hideOverlay() { overlay.classList.add("hidden"); }

function celebrate(rows) {
  if (!rows.length) return;
  const messages = ["Tuyệt!", "Ngon luôn!", "AMAZING!", "PERFECT!"];
  const message = document.createElement("div");
  message.className = `popup popup-${rows.length}`;
  message.textContent = messages[Math.min(rows.length - 1, messages.length - 1)];
  effectsElement.append(message);
  rows.forEach((row) => {
    for (let index = 0; index < WIDTH - 2; index += 1) {
      const particle = document.createElement("i");
      particle.className = "particle";
      particle.style.setProperty("--x", `${(index - 6) * 18}px`);
      particle.style.setProperty("--y", `${(row - 9) * 28}px`);
      particle.style.setProperty("--delay", `${Math.random() * 120}ms`);
      effectsElement.append(particle);
    }
  });
  window.setTimeout(() => { message.remove(); effectsElement.querySelectorAll(".particle").forEach((item) => item.remove()); }, 800);
}

function action(command) {
  if (game.over || game.paused) return;
  let rows = null;
  if (command === "left" && game.move(-1, 0)) playSound("move");
  if (command === "right" && game.move(1, 0)) playSound("move");
  if (command === "down" && game.move(0, 1)) playSound("move");
  if (command === "rotate" && game.rotate()) playSound("rotate");
  if (command === "drop") {
    rows = game.hardDrop();
    playSound("drop");
  }
  if (rows) {
    playSound("lineClear");
    celebrate(rows);
  }
  draw();
  if (game.over) {
    playSound("gameOver");
    showOverlay("Game over", "NICE RUN", "Restart");
  }
}

document.addEventListener("keydown", (event) => {
  const commands = {
    ArrowLeft: "left", a: "left", A: "left",
    ArrowRight: "right", d: "right", D: "right",
    ArrowDown: "down", s: "down", S: "down",
    ArrowUp: "rotate", w: "rotate", W: "rotate",
    " ": "drop", x: "drop", X: "drop",
  };
  if (commands[event.key]) { event.preventDefault(); action(commands[event.key]); }
  if (event.key.toLowerCase() === "p") togglePause();
});

function togglePause() {
  if (game.over) return;
  game.paused = !game.paused;
  if (game.paused) showOverlay("Paused", "TAKE A BREATH", "Resume");
  else hideOverlay();
  draw();
}

document.querySelector("#pauseButton").addEventListener("click", togglePause);
const soundButton = document.querySelector("#soundButton");
function updateSoundButton() {
  soundButton.textContent = isSoundEnabled() ? "♫" : "×";
  soundButton.setAttribute("aria-label", isSoundEnabled() ? "Mute sound" : "Enable sound");
}
soundButton.addEventListener("click", () => {
  setSoundEnabled(!isSoundEnabled());
  updateSoundButton();
});
document.querySelector("#restartButton").addEventListener("click", () => { game.restart(); hideOverlay(); draw(); });
overlayButton.addEventListener("click", () => {
  if (game.over) game.restart();
  else game.paused = false;
  hideOverlay(); draw();
});

function loop(now) {
  const rows = game.tick(now);
  if (rows) {
    playSound("lineClear");
    celebrate(rows);
  }
  draw();
  if (game.over && overlay.classList.contains("hidden")) showOverlay("Game over", "NICE RUN", "Restart");
  requestAnimationFrame(loop);
}

draw();
updateSoundButton();
requestAnimationFrame(loop);
