---@meta

---Boostio graphics and configuration API
---@class boostio
boostio = {}

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

---Add a note to the sequencer
---@param start_ms number Note start time in milliseconds
---@param pitch number MIDI pitch (0-127)
---@param duration_ms number Note duration in milliseconds
---@param voice number? Voice index (default: 0)
function boostio.addNote(start_ms, pitch, duration_ms, voice) end

---Quit the application
function boostio.quit() end

---Save the current project
function boostio.save() end

---Load a project
function boostio.load() end

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

return boostio
