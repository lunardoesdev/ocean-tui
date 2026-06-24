--[[
  logger.lua — Game Action Logger
  Requires: lua-filesystem (luarocks install luafilesystem)
  Logs game actions to a timestamped file for debugging and analytics.
--]]

local logger = {}
local LOG_DIR = "logs"
local LOG_FILE = LOG_DIR .. "/game.log"

local function ensure_dir()
    local ok, lfs = pcall(require, "lfs")
    if ok and lfs.attributes then
        local attr = lfs.attributes(LOG_DIR)
        if not attr then os.execute("mkdir -p " .. LOG_DIR) end
    elseif not io.open(LOG_DIR) then
        os.execute("mkdir -p " .. LOG_DIR)
    end
end

local function timestamp()
    return os.date("%Y-%m-%d %H:%M:%S")
end

function logger.log(action, player, details)
    ensure_dir()
    local f = io.open(LOG_FILE, "a")
    if not f then
        io.stderr:write("ERROR: cannot open " .. LOG_FILE .. "\n")
        return
    end
    local line = string.format("[%s] %-10s | %-20s | %s",
        timestamp(), action, player or "system", tostring(details))
    f:write(line, "\n")
    f:close()
end

function logger.get_stats()
    ensure_dir()
    local f = io.open(LOG_FILE, "r")
    if not f then return "No log entries yet." end
    local count = 0
    for _ in f:lines() do count = count + 1 end
    f:close()
    return string.format("Total log entries: %d", count)
end

function logger.run()
    print("=== Logger Plugin ===")
    print("Writing to: " .. LOG_FILE)
    logger.log("PLUGIN", "logger", "Plugin executed via runner")
    print(logger.get_stats())
    print("Log entry written successfully.")
end

return logger
