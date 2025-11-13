print("Loading keybindings...")

local data_dir = os.getenv("XDG_DATA_HOME") or (os.getenv("HOME") .. "/.local/share")
local commands_path = data_dir .. "/boostio/commands/init.lua"

dofile(commands_path)

boostio.registerKeybinding("q", "quit", { ctrl = true })
boostio.registerKeybinding("s", "save", { ctrl = true })
boostio.registerKeybinding("o", "load", { ctrl = true })

boostio.registerKeybinding("z", "undo", { ctrl = true })
boostio.registerKeybinding("z", "redo", { ctrl = true, shift = true })
boostio.registerKeybinding("y", "redo", { ctrl = true })

boostio.registerKeybinding("space", "toggle_play")
boostio.registerKeybinding("escape", "stop")

boostio.registerKeybinding("g", "toggle_snap")

boostio.registerKeybinding("equals", "zoom_in")
boostio.registerKeybinding("minus", "zoom_out")
boostio.registerKeybinding("up", "transpose_up", { shift = true })
boostio.registerKeybinding("down", "transpose_down", { shift = true })

print("Keybindings loaded!")
