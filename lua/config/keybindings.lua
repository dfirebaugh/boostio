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
boostio.registerKeybinding("f", "toggle_fold")
boostio.registerKeybinding("h", "toggle_scale_highlighting")

boostio.registerKeybinding("r", "cycle_scale_root", { shift = true })
boostio.registerKeybinding("s", "cycle_scale_type", { shift = true })

boostio.registerKeybinding("t", "toast_test", { ctrl = true, shift = true })

print("Keybindings loaded!")
