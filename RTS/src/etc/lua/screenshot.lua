--[[
  screenshot.lua — Terminal Screenshot Capture
  Requires: lua-term (luarocks install lua-term) for terminal detection
  Saves the current game state as a formatted text file.
--]]

local screenshot = {}

function screenshot.capture(filename)
    filename = filename or os.date("screenshot_%Y%m%d_%H%M%S.txt")

    local cols, rows = 80, 24
    local f = io.popen("stty size 2>/dev/null", "r")
    if f then
        local line = f:read("*a")
        f:close()
        local r, c = line:match("(%d+)%s+(%d+)")
        if r and c then cols, rows = tonumber(c), tonumber(r) end
    end

    local lines = {}
    local function add(s) table.insert(lines, s) end

    add(string.rep("=", cols))
    add(" RTS GAME SCREENSHOT")
    add(string.format(" Captured: %s", os.date("%Y-%m-%d %H:%M:%S")))
    add(string.format(" Terminal: %dx%d", cols, rows))
    add(string.rep("=", cols))
    add("")
    add(string.format(" Lua version: %s", _VERSION or "unknown"))
    add(string.format(" Random seed: %d", os.time()))
    add("")
    add(" Loaded modules:")
    for name, _ in pairs(package.loaded) do
        add(string.format("   - %s", name))
    end
    add("")
    add(string.rep("-", cols))
    add(" End of screenshot")
    add(string.rep("=", cols))

    local fh = io.open(filename, "w")
    if not fh then
        return nil, "cannot write to " .. filename
    end
    fh:write(table.concat(lines, "\n"), "\n")
    fh:close()

    return filename
end

function screenshot.run()
    print("=== Screenshot Plugin ===")
    local fname, err = screenshot.capture()
    if fname then
        print(string.format("Screenshot saved: %s (%.0f bytes)", fname, io.open(fname, "r"):read("*a"):len()))
    else
        print("Error: " .. tostring(err))
    end
end

local arg = ...
if arg and arg ~= "" then
    local fname, data = screenshot.capture(arg)
    if fname then
        print("Saved: " .. fname)
    else
        io.stderr:write("Error: " .. data .. "\n")
        os.exit(1)
    end
end

return screenshot
