--[[
  runner.lua — Universal Plugin Runner
  Usage: lua5.4 runner.lua <plugin_path> [args...]
  Loads a plugin module and calls its run() function
  with any additional arguments passed from the command line.
--]]

local args = {...}
local path = args[1]

if not path or path == "" then
    io.stderr:write("Usage: lua5.4 runner.lua <plugin_path> [args...]\n")
    os.exit(1)
end

local ok, plugin = pcall(dofile, path)
if not ok then
    io.stderr:write("Error loading plugin: " .. tostring(plugin) .. "\n")
    os.exit(1)
end

if type(plugin) ~= "table" then
    io.stderr:write("Plugin must return a table (got " .. type(plugin) .. ")\n")
    os.exit(1)
end

if type(plugin.run) ~= "function" then
    io.stderr:write("Plugin has no run() function\n")
    os.exit(1)
end

-- Forward remaining args to plugin.run() as a single string
local input = #args > 1 and table.concat(args, " ", 2) or ""
local ok, err = pcall(plugin.run, input)
if not ok then
    io.stderr:write("Plugin error: " .. tostring(err) .. "\n")
    os.exit(1)
end
