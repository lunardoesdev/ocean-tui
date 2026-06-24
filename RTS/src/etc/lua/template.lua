--[[
  template.lua — Plugin Development Template
  Use this file as a starting point for your own RTS plugins.
  Compatible with Lua 5.1.
--]]

local plugin = {
    name        = "My Plugin",
    version     = "1.0.0",
    author      = "Your Name",
    description = "Short description of what this plugin does",
    api_version = 1,
}

local config = {
    enabled   = true,
    debug     = false,
    log_file  = nil,
}

local function dbg(...)
    if config.debug then
        io.stderr:write(string.format("[DEBUG] [%s] ", plugin.name), ...)
        io.stderr:write("\n")
    end
end

local function log(...)
    local msg = string.format("[%s] ", plugin.name) .. string.format(...)
    print(msg)
    if config.log_file then
        local f = io.open(config.log_file, "a")
        if f then
            f:write(os.date("%Y-%m-%d %H:%M:%S "), msg, "\n")
            f:close()
        end
    end
end

function plugin.on_load()
    dbg("version %s loaded", plugin.version)
    log("Plugin '%s' v%s initialized", plugin.name, plugin.version)
    return true
end

function plugin.on_unit_built(player, unit_type, x, y)
    log("Player %s built a %s at (%d,%d)", player, unit_type, x, y)
end

function plugin.on_combat(attacker, defender, damage)
    dbg("combat: %s -> %s dmg=%d", attacker, defender, damage)
end

function plugin.handle_command(args)
    if not args or #args == 0 then
        log("Usage: :" .. plugin.name:lower() .. " <command>")
        return
    end
    local cmd = table.remove(args, 1)
    if cmd == "status" then
        print(string.format("Plugin: %s v%s by %s", plugin.name, plugin.version, plugin.author))
        print(string.format("Enabled: %s | Debug: %s", config.enabled, config.debug))
    elseif cmd == "help" then
        print(plugin.description)
        print("Commands: status, help")
    else
        log("Unknown command: " .. cmd)
    end
end

function plugin.run()
    print("=== " .. plugin.name .. " v" .. plugin.version .. " ===")
    print("Author: " .. plugin.author)
    print("Description: " .. plugin.description)
    print("")
    plugin.on_load()
    print("")
    print("This is a template plugin. Edit lua/template.lua")
    print("to add your own functionality.")
    print("")
    print("Hooks available:")
    print("  on_load()       - Called when plugin is loaded")
    print("  on_unit_built() - Called when a unit is built")
    print("  on_combat()     - Called on combat")
    print("  handle_command()- Handle custom commands")
    print("  run()           - Called by plugin runner")
end

plugin.on_load()

return plugin
