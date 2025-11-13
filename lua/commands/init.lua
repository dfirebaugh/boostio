local commands_dir = debug.getinfo(1, "S").source:match("@(.*/)")

package.path = commands_dir .. "?.lua;" .. package.path

require("file")
require("edit")
require("playback")
require("grid")
require("view")
require("transform")
require("toast_test")
