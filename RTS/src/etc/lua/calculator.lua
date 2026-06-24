--[[
  calculator.lua — Simple Lua Calculator (like bc)
  Pure Lua 5.1 – no external dependencies.
  Supports: + - * / ^ % sqrt sin cos tan log
--]]

local calculator = {}

local load_fn = loadstring or load

local function evaluate(expr)
    local f, err = load_fn("return (" .. expr .. ")")
    if not f then return nil, "parse error: " .. err end
    local ok, result = pcall(f)
    if not ok then return nil, "runtime error: " .. result end
    return result
end

local function trim(s)
    return s:match("^%s*(.-)%s*$") or s
end

local funcs = {sqrt="math.sqrt", sin="math.sin", cos="math.cos",
               tan="math.tan",   log="math.log", abs="math.abs"}

function calculator.calc(expr)
    expr = trim(expr)
    if expr == "" then return nil, "empty expression" end

    for short, full in pairs(funcs) do
        expr = expr:gsub(short .. "%s*%(", full .. "(")
    end
    expr = expr:gsub("pi", "math.pi")
    expr = expr:gsub("([^%w])e([^%w])", "%1math.exp(1)%2")
    expr = expr:gsub("^e([^%w])", "math.exp(1)%1")
    expr = expr:gsub("([^%w])e$", "%1math.exp(1)")
    expr = expr:gsub("^e$", "math.exp(1)")

    return evaluate(expr)
end

function calculator.run()
    print("=== Calculator Plugin ===")
    print("Evaluates math expressions.")
    print("")
    local examples = {
        "2 + 2 * 5",
        "sqrt(144)",
        "sin(pi / 2)",
        "2 ^ 10",
        "(3 + 5) * 2",
    }
    for _, ex in ipairs(examples) do
        local result, err = calculator.calc(ex)
        if result then
            print(string.format("  %-20s = %g", ex, result))
        end
    end
end

local arg = ...
if arg and arg ~= "" then
    local result, err = calculator.calc(arg)
    if result then
        print(result)
    else
        io.stderr:write("Error: " .. err .. "\n")
        os.exit(1)
    end
end

return calculator
