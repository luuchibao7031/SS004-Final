const SOUND_STORAGE_KEY = "block-bloom-sound-enabled";
const sounds = {
  move: new Audio("./assets/sounds/move.ogg"),
  rotate: new Audio("./assets/sounds/rotate.ogg"),
  drop: new Audio("./assets/sounds/drop.ogg"),
  lineClear: new Audio("./assets/sounds/line-clear.ogg"),
  gameOver: new Audio("./assets/sounds/game-over.ogg"),
};

let enabled = localStorage.getItem(SOUND_STORAGE_KEY) !== "false";

for (const sound of Object.values(sounds)) {
  sound.preload = "auto";
  sound.volume = 0.3;
}

export function isSoundEnabled() {
  return enabled;
}

export function setSoundEnabled(value) {
  enabled = value;
  localStorage.setItem(SOUND_STORAGE_KEY, String(enabled));
}

export function playSound(name) {
  if (!enabled || !sounds[name]) return;

  const sound = sounds[name];
  sound.currentTime = 0;
  sound.play().catch((error) => {
    console.debug(`Unable to play ${name} sound:`, error);
  });
}
