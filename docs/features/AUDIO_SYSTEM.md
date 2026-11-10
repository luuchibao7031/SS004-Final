# Audio system

The browser presentation uses `app/sound.js` and five local OGG files:
movement, rotation, hard drop, line clear, and game over. Audio is played only
after a user action or a game event; it is never fetched from a remote server.

The header button toggles sound and persists the preference in
`block-bloom-sound-enabled`. Browser autoplay restrictions are handled by
logging a debug message when playback is rejected; gameplay continues normally.

The files are from the Kenney Interface Sounds set and retain the source
license at `app/assets/sounds/SOURCE-LICENSE.txt`.
