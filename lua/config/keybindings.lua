print("Loading keybindings...")

local binary_dir = os.getenv("BOOSTIO_BINARY_DIR")
local commands_path

if binary_dir then
	commands_path = binary_dir .. "/commands/init.lua"
else
	local data_dir = os.getenv("XDG_DATA_HOME") or (os.getenv("HOME") .. "/.local/share")
	commands_path = data_dir .. "/boostio/commands/init.lua"
end

dofile(commands_path)

boostio.registerKeybinding("q", "quit", { ctrl = true })
boostio.registerKeybinding("s", "save", { ctrl = true })

boostio.registerKeybinding("z", "undo", { ctrl = true })
boostio.registerKeybinding("z", "redo", { ctrl = true, shift = true })
boostio.registerKeybinding("y", "redo", { ctrl = true })

boostio.registerKeybinding("delete", "delete_selected")
boostio.registerKeybinding("backspace", "delete_selected")

boostio.registerKeybinding("space", "toggle_play")
boostio.registerKeybinding("escape", "stop")

boostio.registerKeybinding("[", "decrease_bpm")
boostio.registerKeybinding("[", "decrease_bpm_5", { ctrl = true })
boostio.registerKeybinding("[", "decrease_bpm_10", { shift = true })

boostio.registerKeybinding("]", "increase_bpm")
boostio.registerKeybinding("]", "increase_bpm_5", { ctrl = true })
boostio.registerKeybinding("]", "increase_bpm_10", { shift = true })

boostio.registerKeybinding("g", "toggle_snap")
boostio.registerKeybinding("f", "toggle_fold")
boostio.registerKeybinding("h", "toggle_scale_highlighting")

boostio.registerKeybinding("r", "cycle_scale_root", { shift = true })
boostio.registerKeybinding("s", "cycle_scale_type", { shift = true })

boostio.registerKeybinding("t", "toast_test", { ctrl = true, shift = true })

print("Keybindings loaded!")
