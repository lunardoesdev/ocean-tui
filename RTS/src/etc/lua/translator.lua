--[[
  translator.lua — Simple Dictionary Translator (ES ↔ RU ↔ EN)
  Requires: lua-cjson (luarocks install lua-cjson) for JSON import/export.
--]]

local translator = {}

local en_to_es = {
    attack = "ataque",    build = "construir",  gold = "oro",
    income = "ingreso",   unit = "unidad",      ground = "terrestre",
    naval = "naval",      air = "aéreo",        city = "ciudad",
    forest = "bosque",    mountain = "montaña", water = "agua",
    plains = "llanura",   economy = "economía", help = "ayuda",
    pause = "pausa",      quit = "salir",       victory = "victoria",
    defeat = "derrota",   soldier = "soldado",  castle = "castillo",
    battle = "batalla",   peace = "paz",        war = "guerra",
    power = "poder",      speed = "velocidad",  damage = "daño",
    health = "salud",
}

local en_to_ru = {
    attack = "атака",     build = "строить",    gold = "золото",
    income = "доход",     unit = "юнит",        ground = "наземный",
    naval = "морской",    air = "воздушный",    city = "город",
    forest = "лес",       mountain = "гора",    water = "вода",
    plains = "равнина",   economy = "экономика", help = "помощь",
    pause = "пауза",      quit = "выход",       victory = "победа",
    defeat = "поражение", soldier = "солдат",   castle = "замок",
    battle = "битва",     peace = "мир",        war = "война",
    power = "сила",       speed = "скорость",   damage = "урон",
    health = "здоровье",
}

function translator.translate(word, target_lang)
    local word_lower = word:lower()
    local dict
    if target_lang == "es" or target_lang == "spanish" then
        dict = en_to_es
    elseif target_lang == "ru" or target_lang == "russian" then
        dict = en_to_ru
    else
        return nil, "unsupported language: " .. target_lang
    end
    local translation = dict[word_lower]
    if translation then return translation end
    for eng, trans in pairs(dict) do
        if word_lower:find(eng) or eng:find(word_lower) then
            return trans .. " (maybe)"
        end
    end
    return nil, "word '" .. word .. "' not found"
end

function translator.run()
    print("=== Translator Plugin ===")
    print("")
    local tests = {
        {"attack", "es"},
        {"build", "es"},
        {"gold", "ru"},
        {"economy", "ru"},
        {"victory", "spanish"},
    }
    for _, t in ipairs(tests) do
        local result, err = translator.translate(t[1], t[2])
        if result then
            print(string.format("  %s (%s) → %s", t[1], t[2], result))
        else
            print(string.format("  %s (%s) → ERROR: %s", t[1], t[2], err))
        end
    end
end

return translator
