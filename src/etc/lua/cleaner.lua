--[[
  cleaner.lua — Project Line Counter & .o Cleaner
  Scans .cpp, .h, .lua, .cfg, .md, Makefile and totals lines.
  Removes all .o object files after compilation.
--]]

local cleaner = {}

local function trim(s)
    return s:match("^%s*(.-)%s*$") or s
end

function cleaner.count_lines(dir)
    dir = dir or "."
    local total_lines = 0
    local file_counts = {}
    local extensions = {cpp=true, h=true, lua=true, cfg=true, md=true}
    local single_files = {["Makefile"]=true, ["notes.md"]=true}

    local function scan(path)
        local f = io.popen("find " .. path .. " -type f 2>/dev/null", "r")
        if not f then return end
        for file in f:lines() do
            if file:match("%.o$") then goto skip end  -- skip .o files

            local ext = file:match("%.([^%.]+)$")
            local basename = file:match("/([^/]+)$")

            if (ext and extensions[ext:lower()]) or (basename and single_files[basename]) then
                local fh = io.open(file, "r")
                if fh then
                    local count = 0
                    for _ in fh:lines() do count = count + 1 end
                    fh:close()
                    total_lines = total_lines + count
                    local display = file:gsub("^%.%./", "")
                    table.insert(file_counts, {file=display, lines=count})
                end
            end
            ::skip::
        end
        f:close()
    end

    scan(dir)

    table.sort(file_counts, function(a, b) return a.lines > b.lines end)

    print(string.format("=== Project Line Count ==="))
    print(string.format("Total lines: %d", total_lines))
    print("")
    print(string.format("%-50s %s", "File", "Lines"))
    print(string.rep("-", 62))
    for _, fc in ipairs(file_counts) do
        print(string.format("%-50s %5d", fc.file, fc.lines))
    end

    return total_lines, file_counts
end

function cleaner.remove_objects()
    print("")
    print("=== Removing .o files ===")
    local f = io.popen("find . -name '*.o' -type f 2>/dev/null", "r")
    if not f then
        print("Error: cannot scan for .o files")
        return 0
    end

    local count = 0
    for file in f:lines() do
        local ok = os.remove(file)
        if ok then
            print("  Removed: " .. file)
            count = count + 1
        else
            print("  Failed: " .. file)
        end
    end
    f:close()

    if count == 0 then
        print("  No .o files found.")
    else
        print(string.format("  Removed %d object file(s).", count))
    end
    return count
end

function cleaner.run()
    print("=== Cleaner Plugin ===")
    print("")
    print("Step 1: Counting lines of code...")
    print("")
    cleaner.count_lines(".")
    print("")
    print("Step 2: Cleaning object files...")
    cleaner.remove_objects()
    print("")
    print("Done.")
end

return cleaner
