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

return boostio
