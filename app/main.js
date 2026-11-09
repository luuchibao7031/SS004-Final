import { Game, HEIGHT, WIDTH, SIZE } from "./game.js";
import { isSoundEnabled, playSound, setSoundEnabled } from "./sound.js";

const game = new Game("classic");
const boardElement = document.querySelector("#board");
const nextElement = document.querySelector("#next");
const effectsElement = document.querySelector("#effects");
const overlay = document.querySelector("#overlay");
const overlayTitle = document.querySelector("#overlayTitle");
const overlayEyebrow = document.querySelector("#overlayEyebrow");
const overlayButton = document.querySelector("#overlayButton");
const highScoreElement = document.querySelector("#highScore");
let highScore = Number(localStorage.getItem("block-bloom-high-score") || 0);
let clearFlash = null;
let menuIsInitial = true;
const modeMenu = document.querySelector("#modeMenu");
const modeLabel = document.querySelector("#modeLabel");
const sprintTimer = document.querySelector("#sprintTimer");
const modeButton = document.querySelector("#modeButton");
const closeModeMenu = document.querySelector("#closeModeMenu");

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
      cell.className = `mini-cell${type === " " ? "" : ` piece-${type}`}`;
      if (type !== " ") cell.dataset.piece = type;
      target.append(cell);
    }
  }
}

function draw(now = performance.now()) {
  const flashAge = clearFlash ? now - clearFlash.started : Number.POSITIVE_INFINITY;
  const flashing = clearFlash && flashAge < clearFlash.duration;
  if (clearFlash && !flashing) clearFlash = null;
  const shouldShowFlash = flashing && Math.floor(flashAge / 100) % 2 === 0;
  const source = shouldShowFlash ? clearFlash.snapshot : game.board.cells;
  const visible = source.map((row) => [...row]);
  if (game.mode === "invisible" && !shouldShowFlash) {
    for (let row = 1; row < HEIGHT - 1; row += 1) {
      for (let column = 1; column < WIDTH - 1; column += 1) visible[row][column] = " ";
    }
  }
  if (game.mode !== "invisible") {
    const ghostY = game.y + game.board.dropDistance(game.active, game.x, game.y);
    game.active.cells.forEach((row, rowIndex) => row.forEach((type, columnIndex) => {
      if (type !== " ") visible[ghostY + rowIndex][game.x + columnIndex] = `ghost-${type}`;
    }));
  }
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
  modeLabel.textContent = game.mode === "sprint" ? `SPRINT · ${game.sprintTarget} LINES` :
    game.mode === "invisible" ? "INVISIBLE · GHOST OFF" : "CLASSIC · GHOST ON";
  sprintTimer.classList.toggle("hidden", game.mode !== "sprint");
  if (game.mode === "sprint") {
    const elapsed = Math.max(0, game.elapsedMs);
    sprintTimer.textContent = `${String(Math.floor(elapsed / 60000)).padStart(2, "0")}:${String(Math.floor(elapsed / 1000) % 60).padStart(2, "0")}.${String(Math.floor(elapsed % 1000 / 100))}`;
  }
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

function handleClear(rows) {
  if (!rows || rows.length === 0) return;
  playSound("lineClear");
  if (game.mode === "invisible" && game.lastClearSnapshot) {
    clearFlash = { snapshot: game.lastClearSnapshot, started: performance.now(), duration: 700 };
  }
  celebrate(rows);
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
  handleClear(rows);
  draw();
  if (game.over) {
    playSound("gameOver");
    showOverlay("Game over", "NICE RUN", "Restart");
  }
  if (game.finished) showOverlay("Sprint complete!", `${game.sprintTarget} LINES CLEARED`, "New run");
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
function openModeMenu(initial = false) {
  menuIsInitial = initial;
  closeModeMenu.classList.toggle("hidden", initial);
  modeMenu.classList.remove("hidden");
  game.paused = true;
}
function startMode(mode) {
  game.restart(mode);
  game.paused = false;
  game.startedAt = performance.now();
  clearFlash = null;
  modeMenu.classList.add("hidden");
  hideOverlay();
  draw();
}
document.querySelectorAll("[data-mode]").forEach((button) => {
  button.addEventListener("click", () => startMode(button.dataset.mode));
});
closeModeMenu.addEventListener("click", () => {
  if (!menuIsInitial) {
    modeMenu.classList.add("hidden");
    game.paused = false;
  }
});
modeButton.addEventListener("click", () => openModeMenu(false));
document.querySelector("#restartButton").addEventListener("click", () => startMode(game.mode));
overlayButton.addEventListener("click", () => {
  if (game.over || game.finished) game.restart(game.mode);
  else game.paused = false;
  hideOverlay(); draw();
});

function loop(now) {
  const rows = game.tick(now);
  handleClear(rows);
  draw(now);
  if (game.over && overlay.classList.contains("hidden")) showOverlay("Game over", "NICE RUN", "Restart");
  if (game.finished && overlay.classList.contains("hidden")) showOverlay("Sprint complete!", `${game.sprintTarget} LINES CLEARED`, "New run");
  requestAnimationFrame(loop);
}

draw();
updateSoundButton();
openModeMenu(true);
requestAnimationFrame(loop);
