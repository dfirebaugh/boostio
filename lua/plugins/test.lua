local test = {}

function test.render()
	boostio.drawRectangle(200, 200, 400, 200, 1.0, 0.0, 0.0, 1.0)
	boostio.drawText("HELLO FROM LUA!", 250, 280, 32, 1.0, 1.0, 0.0, 1.0)
end

return test
