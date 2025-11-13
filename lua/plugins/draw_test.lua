local draw_test = {}

function draw_test.render()
	boostio.drawRectangle(10, 10, 100, 50, 0.2, 0.4, 0.8, 1.0)
	boostio.drawRectangleOutlined(120, 10, 100, 50, 0.8, 0.2, 0.2, 1.0, 1.0, 1.0, 1.0, 1.0, 2)
	boostio.strokeRectangle(230, 10, 100, 50, 0.2, 0.8, 0.2, 1.0)
	boostio.drawRoundedRectangle(10, 70, 100, 50, 10, 0.8, 0.4, 0.2, 1.0)
	boostio.drawRoundedRectangleOutlined(120, 70, 100, 50, 10, 0.2, 0.8, 0.8, 1.0, 1.0, 1.0, 1.0, 1.0, 2)
	boostio.strokeRoundedRectangle(230, 70, 100, 50, 10, 0.8, 0.2, 0.8, 1.0)
	boostio.drawLine(10, 140, 100, 140, 1.0, 1.0, 0.0, 1.0)
	boostio.drawLine(10, 150, 100, 180, 0.0, 1.0, 1.0, 1.0)
	boostio.drawLine(100, 150, 10, 180, 1.0, 0.0, 1.0, 1.0)

	local text = "Drawing API Test"
	local width = boostio.measureText(text, 20)
	boostio.drawText(text, 10, 200, 20, 1.0, 1.0, 1.0, 1.0)
	boostio.drawText(string.format("width: %.1f", width), 10, 230, 14, 0.7, 0.7, 0.7, 1.0)
end

return draw_test
