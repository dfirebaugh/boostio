---@meta

---Boostio graphics and configuration API
---@class boostio
boostio = {}

---Waveform types for audio synthesis
boostio.WAVEFORM_SINE = 0
boostio.WAVEFORM_SQUARE = 1
boostio.WAVEFORM_TRIANGLE = 2
boostio.WAVEFORM_SAWTOOTH = 3
boostio.WAVEFORM_NES_NOISE = 4

---Mouse button constants
boostio.MOUSE_BUTTON_LEFT = 1
boostio.MOUSE_BUTTON_MIDDLE = 2
boostio.MOUSE_BUTTON_RIGHT = 3

---Draw a filled rectangle
---@param x number X position
---@param y number Y position
---@param width number Rectangle width
---@param height number Rectangle height
---@param r number Red component (0.0-1.0)
---@param g number Green component (0.0-1.0)
---@param b number Blue component (0.0-1.0)
---@param a number? Alpha component (0.0-1.0, default: 1.0)
function boostio.drawRectangle(x, y, width, height, r, g, b, a) end

---Draw a filled rectangle with an outline
---@param x number X position
---@param y number Y position
---@param width number Rectangle width
---@param height number Rectangle height
---@param r number Fill red component (0.0-1.0)
---@param g number Fill green component (0.0-1.0)
---@param b number Fill blue component (0.0-1.0)
---@param a number? Fill alpha component (0.0-1.0, default: 1.0)
---@param or number Outline red component (0.0-1.0)
---@param og number Outline green component (0.0-1.0)
---@param ob number Outline blue component (0.0-1.0)
---@param oa number? Outline alpha component (0.0-1.0, default: 1.0)
---@param outline_width number Outline width in pixels
function boostio.drawRectangleOutlined(x, y, width, height, r, g, b, a, or, og, ob, oa, outline_width) end

---Draw a rectangle outline (stroke only, no fill)
---@param x number X position
---@param y number Y position
---@param width number Rectangle width
---@param height number Rectangle height
---@param r number Red component (0.0-1.0)
---@param g number Green component (0.0-1.0)
---@param b number Blue component (0.0-1.0)
---@param a number? Alpha component (0.0-1.0, default: 1.0)
function boostio.strokeRectangle(x, y, width, height, r, g, b, a) end

---Draw a filled rounded rectangle
---@param x number X position
---@param y number Y position
---@param width number Rectangle width
---@param height number Rectangle height
---@param radius number Corner radius in pixels
---@param r number Red component (0.0-1.0)
---@param g number Green component (0.0-1.0)
---@param b number Blue component (0.0-1.0)
---@param a number? Alpha component (0.0-1.0, default: 1.0)
function boostio.drawRoundedRectangle(x, y, width, height, radius, r, g, b, a) end

---Draw a filled rounded rectangle with an outline
---@param x number X position
---@param y number Y position
---@param width number Rectangle width
---@param height number Rectangle height
---@param radius number Corner radius in pixels
---@param r number Fill red component (0.0-1.0)
---@param g number Fill green component (0.0-1.0)
---@param b number Fill blue component (0.0-1.0)
---@param a number? Fill alpha component (0.0-1.0, default: 1.0)
---@param or number Outline red component (0.0-1.0)
---@param og number Outline green component (0.0-1.0)
---@param ob number Outline blue component (0.0-1.0)
---@param oa number? Outline alpha component (0.0-1.0, default: 1.0)
---@param outline_width number Outline width in pixels
function boostio.drawRoundedRectangleOutlined(x, y, width, height, radius, r, g, b, a, or, og, ob, oa, outline_width) end

---Draw a rounded rectangle outline (stroke only, no fill)
---@param x number X position
---@param y number Y position
---@param width number Rectangle width
---@param height number Rectangle height
---@param radius number Corner radius in pixels
---@param r number Red component (0.0-1.0)
---@param g number Green component (0.0-1.0)
---@param b number Blue component (0.0-1.0)
---@param a number? Alpha component (0.0-1.0, default: 1.0)
function boostio.strokeRoundedRectangle(x, y, width, height, radius, r, g, b, a) end

---Draw a line
---@param x1 number Start X position
---@param y1 number Start Y position
---@param x2 number End X position
---@param y2 number End Y position
---@param r number Red component (0.0-1.0)
---@param g number Green component (0.0-1.0)
---@param b number Blue component (0.0-1.0)
---@param a number? Alpha component (0.0-1.0, default: 1.0)
function boostio.drawLine(x1, y1, x2, y2, r, g, b, a) end

---Draw text at the specified position
---@param text string Text to draw
---@param x number X position
---@param y number Y position
---@param size number Font size in pixels
---@param r number Red component (0.0-1.0)
---@param g number Green component (0.0-1.0)
---@param b number Blue component (0.0-1.0)
---@param a number? Alpha component (0.0-1.0, default: 1.0)
function boostio.drawText(text, x, y, size, r, g, b, a) end

---Measure the width of text in pixels
---@param text string Text to measure
---@param size number Font size in pixels
---@return number width Width of the text in pixels
function boostio.measureText(text, size) end

---Convert hex color string to RGB table
---@param hex string Hex color string (e.g., "#FF0000" or "FF0000")
---@return table rgb Table with r, g, b fields (0.0-1.0)
function boostio.hexToRgb(hex) end

---Check if a point is inside a rectangle
---@param px number Point X coordinate
---@param py number Point Y coordinate
---@param x number Rectangle X coordinate
---@param y number Rectangle Y coordinate
---@param width number Rectangle width
---@param height number Rectangle height
---@return boolean inside True if point is inside rectangle
function boostio.isPointInRect(px, py, x, y, width, height) end

---Get the current window dimensions
---@return number width Window width in pixels
---@return number height Window height in pixels
function boostio.getWindowSize() end

---Get the current frames per second (FPS)
---@return number fps Current FPS
function boostio.getFps() end

---Get the current mouse position
---@return number x Mouse X position in pixels
---@return number y Mouse Y position in pixels
function boostio.getMousePosition() end

---Check if a mouse button is currently held down
---@param button number Mouse button (use boostio.MOUSE_BUTTON_LEFT, MIDDLE, or RIGHT)
---@return boolean down True if the button is currently pressed
function boostio.isMouseButtonDown(button) end

---Check if a mouse button was just pressed this frame
---@param button number Mouse button (use boostio.MOUSE_BUTTON_LEFT, MIDDLE, or RIGHT)
---@return boolean pressed True if the button was just pressed (only true for one frame)
function boostio.isMouseButtonPressed(button) end

---Get a configuration value
---@param key string Configuration key (e.g., "window.width")
---@return any value The configuration value, or nil if not found
function boostio.getConfig(key) end

---Register a custom Lua command
---@param name string Command name
---@param callback function Function to call when command is executed
function boostio.registerCommand(name, callback) end

---@class KeybindingModifiers
---@field shift boolean? Shift key modifier (default: false)
---@field ctrl boolean? Control key modifier (default: false)
---@field alt boolean? Alt key modifier (default: false)

---Register a keybinding
---@param key string Key name: "a"-"z", "0"-"9", "space", "escape", "enter", "tab", "f1"-"f12", "up", "down", "left", "right", "minus", "equals"
---@param command_name string Name of the command to execute
---@param modifiers KeybindingModifiers? Optional modifier keys (shift, ctrl, alt)
function boostio.registerKeybinding(key, command_name, modifiers) end

---Set the BPM (beats per minute)
---@param bpm number BPM value
function boostio.setBpm(bpm) end

---Set the playhead position
---@param position_ms number Position in milliseconds
function boostio.setPlayhead(position_ms) end

---@class NoteParams
---@field waveform number? Waveform type (default: WAVEFORM_SINE)
---@field duty_cycle number? Square wave duty cycle 0-255 (default: 128 = 50%)
---@field decay number? Decay rate -32768 to 32767, positive=fade out, negative=fade in (default: 0)
---@field amplitude_dbfs number? Amplitude in dBFS -128 to 0 (default: -3)
---@field nes_noise_period number? NES noise period index 0-15 (default: 15)
---@field nes_noise_mode_flag boolean? NES noise mode: true=tonal, false=white noise (default: false)

---Add a note to the sequencer with the currently selected instrument settings
---Uses the selected voice and selected instrument's parameters by default
---@param start_ms number Note start time in milliseconds
---@param pitch number MIDI pitch (0-127, where 60 is middle C, 69 is A4 at 440Hz)
---@param duration_ms number Note duration in milliseconds
---@param params number|NoteParams? Either waveform type or table with advanced parameters (can include voice field to override selected voice)
---@return integer note_id The unique ID of the created note
function boostio.addNote(start_ms, pitch, duration_ms, params) end

---Quit the application
function boostio.quit() end

---Save the current project in all formats (.json, .c, .wav)
---If a file was previously loaded, saves to that location
---If filepath is provided, saves to that location and sets it as current
---Automatically generates .json, .c, and .wav files from the base path
---@param filepath string? Optional filepath (default: current file or "song.json")
---@return boolean success True if all formats saved successfully
function boostio.save(filepath) end

---Export only the C code format (for individual export)
---@param filepath string? Optional filepath (default: "song.c")
---@return boolean success True if export succeeded
function boostio.saveC(filepath) end

---Export only the WAV audio format (for individual export)
---@param filepath string? Optional filepath (default: "song.wav")
---@return boolean success True if export succeeded
function boostio.saveWav(filepath) end

---Load a project from JSON file
---Sets the loaded file as the current file for future saves
---@param filepath string? Optional filepath (default: "song.json")
---@return boolean success True if load succeeded
function boostio.load(filepath) end

---Undo the last action
function boostio.undo() end

---Redo the last undone action
function boostio.redo() end

---Toggle play/pause
function boostio.togglePlay() end

---Start playback
function boostio.play() end

---Pause playback
function boostio.pause() end

---Stop playback
function boostio.stop() end

---Toggle grid snapping
function boostio.toggleSnap() end

---Zoom in
function boostio.zoomIn() end

---Zoom out
function boostio.zoomOut() end

---Transpose selected notes up
function boostio.transposeUp() end

---Transpose selected notes down
function boostio.transposeDown() end

---Toggle scale highlighting on/off
---@return boolean enabled New state of scale highlighting
function boostio.toggleScaleHighlight() end

---Cycle to the next scale type
---@return string scale_name Name of the new scale (e.g., "major", "minor")
function boostio.cycleScaleType() end

---Cycle to the next root note
---@return string root_name Name of the new root note (e.g., "C", "D#")
function boostio.cycleScaleRoot() end

---Get current scale information
---@return table info Table with fields: scale (string), root (string), highlight (boolean)
function boostio.getScaleInfo() end

---Toggle fold mode on/off (hides notes not in the selected scale)
---@return boolean enabled New state of fold mode
function boostio.toggleFold() end

---Get comprehensive app state information
---@return table state Table with fields: bpm, selected_voice, waveform, note_count, is_playing, playhead_ms, snap_enabled, snap_ms, selected_scale, selected_root, voice_hidden, voice_solo, voice_muted, viewport, notes, fold_mode, show_scale_highlights
function boostio.getAppState() end

---Set whether a voice is hidden (not displayed visually)
---@param voice integer Voice index (0-7)
---@param hidden boolean Whether the voice should be hidden
function boostio.setVoiceHidden(voice, hidden) end

---Set whether a voice is solo'd (mutes all other non-solo voices)
---@param voice integer Voice index (0-7)
---@param solo boolean Whether the voice should be solo'd
function boostio.setVoiceSolo(voice, solo) end

---Set whether a voice is muted
---@param voice integer Voice index (0-7)
---@param muted boolean Whether the voice should be muted
function boostio.setVoiceMuted(voice, muted) end

---Check if a piano key is in the given scale
---@param piano_key integer Piano key (0-127)
---@param scale string Scale type (e.g., "major", "minor", "chromatic")
---@param root string Root note (e.g., "C", "D#", "A")
---@return boolean in_scale True if the piano key is in the scale
function boostio.isNoteInScale(piano_key, scale, root) end

---Check if a piano key is the root note
---@param piano_key integer Piano key (0-127)
---@param root string Root note (e.g., "C", "D#", "A")
---@return boolean is_root True if the piano key is the root note
function boostio.isRootNote(piano_key, root) end

---Convert milliseconds to X coordinate in the piano roll
---@param ms number Time in milliseconds
---@return number x X coordinate in pixels
function boostio.msToX(ms) end

---Convert X coordinate to milliseconds in the piano roll
---@param x number X coordinate in pixels
---@return number ms Time in milliseconds
function boostio.xToMs(x) end

---Convert piano key to Y coordinate in the piano roll
---@param piano_key integer Piano key (0-127)
---@return number y Y coordinate in pixels
function boostio.pianoKeyToY(piano_key) end

---Convert Y coordinate to piano key in the piano roll
---@param y number Y coordinate in pixels
---@return integer piano_key Piano key (0-127)
function boostio.yToPianoKey(y) end

---Get the list of selected note IDs
---@return table selected_ids Array of selected note IDs
function boostio.getSelection() end

---Clear all note selections
function boostio.clearSelection() end

---Add a note to the selection
---@param note_id integer ID of the note to select
function boostio.selectNote(note_id) end

---Remove a note from the selection
---@param note_id integer ID of the note to deselect
function boostio.deselectNote(note_id) end

---Check if a note is currently selected
---@param note_id integer ID of the note to check
---@return boolean is_selected True if the note is selected
function boostio.isNoteSelected(note_id) end

---Move a note by a delta amount in time and pitch
---@param note_id integer ID of the note to move
---@param delta_ms integer Time delta in milliseconds
---@param delta_piano_key integer Pitch delta in piano keys
function boostio.moveNote(note_id, delta_ms, delta_piano_key) end

---Resize a note by adjusting its duration
---@param note_id integer ID of the note to resize
---@param delta_duration_ms integer Duration delta in milliseconds
---@param from_left boolean True to resize from left edge, false for right edge
function boostio.resizeNote(note_id, delta_duration_ms, from_left) end

---Delete a note from the project
---@param note_id integer ID of the note to delete
function boostio.deleteNote(note_id) end

---Play a preview note using the selected instrument
---@param piano_key integer Piano key to preview (0-127)
function boostio.playPreviewNote(piano_key) end

---Get the number of available instruments
---@return integer count Number of instruments available
function boostio.getInstrumentCount() end

---Get instrument properties by index
---@param index integer Instrument index (0-based)
---@return Instrument? instrument Instrument data or nil if invalid index
function boostio.getInstrument(index) end

---Set the selected instrument for placing new notes
---@param index integer Instrument index (0-based)
function boostio.setSelectedInstrument(index) end

---Set the selected voice for placing new notes
---@param voice integer Voice index (0-7)
function boostio.setSelectedVoice(voice) end

---Set the voice of an existing note
---@param note_id integer ID of the note to modify
---@param voice integer Voice index (0-7)
function boostio.setNoteVoice(note_id, voice) end

---Check if a key is currently pressed down
---@param key string Key name (e.g., "ctrl", "shift", "alt", "a", "space")
---@return boolean is_down True if the key is currently pressed
function boostio.isKeyDown(key) end

---Zoom horizontally (time axis) centered on the mouse cursor position
---@param factor number Zoom factor (>1 zooms in, <1 zooms out)
---@param mouse_x number X position of mouse cursor
function boostio.zoomHorizontalAtMouse(factor, mouse_x) end

---Zoom vertically (pitch axis) centered on the mouse cursor position
---@param factor number Zoom factor (>1 zooms in, <1 zooms out)
---@param mouse_y number Y position of mouse cursor
function boostio.zoomVerticalAtMouse(factor, mouse_y) end

return boostio
