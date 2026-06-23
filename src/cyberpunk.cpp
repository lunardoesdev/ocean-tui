#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <map>
#include <vector>
#include <ctime>
#include <cmath>
#include <thread>
#include <chrono>
#include <atomic>
#include <sstream>
#include <random>
#include <algorithm>
#include <cstdlib>

// ANSI цветовые коды для киберпанка
#define CYAN "\033[36m"
#define MAGENTA "\033[35m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define BLUE "\033[34m"
#define WHITE "\033[37m"
#define BRIGHT_CYAN "\033[96m"
#define BRIGHT_MAGENTA "\033[95m"
#define BRIGHT_GREEN "\033[92m"
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define DIM "\033[2m"
#define BLINK "\033[5m"

struct Config {
    std::string primaryColor;
    std::string secondaryColor;
    int updateRate;
    bool enableAnimation;
    bool enableBlink;
    std::string logoStyle;
    std::vector<std::string> commands;
};

class CyberpunkInterface {
private:
    Config config;
    std::atomic<bool> running{true};
    int frameCount{0};
    int width{80}, height{24};
    int selectedCommand{0};
    std::map<char, int> keyPresses;
    std::vector<std::string> commandResults;
    std::vector<std::string> commandLabels;
    termios oldTermios, newTermios;

    std::vector<std::string> allColors{"cyan", "magenta", "green", "red", "yellow", "blue"};

    std::string getColorCode(const std::string& colorName) {
        std::string name = colorName;
        if (name == "random") {
            static std::mt19937 gen(std::time(nullptr));
            std::uniform_int_distribution<> dis(0, allColors.size() - 1);
            name = allColors[dis(gen)];
        }

        if (name == "cyan") return BRIGHT_CYAN;
        if (name == "magenta") return BRIGHT_MAGENTA;
        if (name == "green") return GREEN;
        if (name == "red") return RED;
        if (name == "yellow") return YELLOW;
        if (name == "blue") return BLUE;
        return CYAN;
    }

    void loadConfig(const std::string& filepath) {
        std::ifstream file(filepath);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;

                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);

                    if (key == "primary_color") config.primaryColor = value;
                    else if (key == "secondary_color") config.secondaryColor = value;
                    else if (key == "update_rate") config.updateRate = std::stoi(value);
                    else if (key == "enable_animation") config.enableAnimation = (value == "true");
                    else if (key == "enable_blink") config.enableBlink = (value == "true");
                    else if (key == "logo_style") config.logoStyle = value;
                    else if (key.find("command_") == 0) {
                        config.commands.push_back(value);
                    }
                }
            }
            file.close();
        }
    }

    void enableRawMode() {
        tcgetattr(STDIN_FILENO, &oldTermios);
        newTermios = oldTermios;
        newTermios.c_lflag &= ~(ICANON | ECHO);
        newTermios.c_cc[VMIN] = 0;
        newTermios.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &newTermios);
    }

    void disableRawMode() {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldTermios);
    }

    char readKey() {
        char c;
        if (read(STDIN_FILENO, &c, 1) > 0) {
            return c;
        }
        return '\0';
    }

    void clearScreen() {
        std::cout << "\033[2J\033[H";
    }

    std::string executeCommand(const std::string& cmd) {
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return "[ERROR]";

        std::string result;
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe)) {
            result += buffer;
        }
        pclose(pipe);

        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
        return result.length() > 60 ? result.substr(0, 57) + "..." : result;
    }

    void executeAllCommands() {
        commandResults.clear();
        commandLabels.clear();
        for (size_t i = 0; i < config.commands.size(); ++i) {
            commandResults.push_back(executeCommand(config.commands[i]));
            commandLabels.push_back("Команда " + std::to_string(i + 1));
        }
    }

    void executeSelectedCommand() {
        if (selectedCommand >= 0 && selectedCommand < static_cast<int>(config.commands.size())) {
            commandResults[selectedCommand] = executeCommand(config.commands[selectedCommand]);
        }
    }

    void drawBorder() {
        std::string primary = getColorCode(config.primaryColor);
        std::string secondary = getColorCode(config.secondaryColor);

        for (int i = 0; i < width; i++) std::cout << primary << "═";
        std::cout << RESET << std::endl;
    }

    void drawLogo() {
        std::string primary = getColorCode(config.primaryColor);
        std::string secondary = getColorCode(config.secondaryColor);

        if (config.logoStyle == "simple") {
            std::cout << primary << BOLD << "╔═══ CYBERPUNK TERMINAL v1.0 ═══╗" << RESET << std::endl;
        } else if (config.logoStyle == "ascii_art") {
            std::cout << primary << BOLD
                << " ▄████████    ▄█  ▀█████████▄   ▄████████ ▀█████████▄" << RESET << std::endl;
            std::cout << secondary << BOLD
                << " ███    ███   ███    ███    ███ ███    ███   ███    ███" << RESET << std::endl;
            std::cout << primary
                << " ███    █▀    ███    ███    ███ ███    ███   ███    ███" << RESET << std::endl;
        } else if (config.logoStyle == "matrix") {
            std::cout << primary << BOLD << "  M A T R I X   T E R M I N A L" << RESET << std::endl;
            std::cout << secondary << "  " << DIM << "████████ ████████ ████████" << RESET << std::endl;
        } else if (config.logoStyle == "neon") {
            std::cout << primary << BOLD << "  ◆━━━ NEON INTERFACE ━━━◆" << RESET << std::endl;
            std::cout << secondary << "  " << BRIGHT_CYAN << "⚡ SYSTEM ONLINE ⚡" << RESET << std::endl;
        }
    }

    void drawStatusBar() {
        std::string primary = getColorCode(config.primaryColor);
        time_t now = time(0);
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime(&now));

        std::cout << primary << "► Статус системы: [";
        std::cout << BRIGHT_GREEN << "●" << primary << "] ";
        std::cout << "Кадр: " << frameCount << " | ";
        std::cout << "Время: " << timeStr << RESET << std::endl;
    }

    void drawDataStream() {
        std::string secondary = getColorCode(config.secondaryColor);

        for (int i = 0; i < 3; i++) {
            int offset = (frameCount + i * 5) % 60;
            std::cout << secondary << DIM;
            for (int j = 0; j < width; j++) {
                if ((j + offset) % 15 == 0) {
                    std::cout << BRIGHT_CYAN << "█" << secondary << DIM;
                } else {
                    std::cout << static_cast<char>(48 + (j + i + frameCount) % 10);
                }
            }
            std::cout << RESET << std::endl;
        }
    }

    void drawKeyInfo() {
        std::string primary = getColorCode(config.primaryColor);
        std::cout << primary << "╔ МОНИТОР ВВОДА ╗" << RESET << std::endl;

        if (keyPresses.empty()) {
            std::cout << BRIGHT_CYAN << "  [Ожидание ввода...]" << RESET << std::endl;
        } else {
            for (const auto& kp : keyPresses) {
                std::cout << primary << "  '" << kp.first << "' → "
                         << BRIGHT_MAGENTA << kp.second << " раз" << RESET << std::endl;
            }
        }
    }

    void drawMenu() {
        std::string primary = getColorCode(config.primaryColor);
        std::cout << primary << "\n╔════════════════════════════════════════╗" << RESET << std::endl;
        std::cout << primary << "║ " << BRIGHT_CYAN << "[Q] Выход " << primary
                 << "[C] Очистить " << BRIGHT_GREEN << "[R] Сброс" << primary << " ║" << RESET << std::endl;
        std::cout << primary << "║ " << BRIGHT_MAGENTA << "[↑/↓] Команда  [ENTER] Запустить" << primary << "    ║" << RESET << std::endl;
        std::cout << primary << "╚════════════════════════════════════════╝" << RESET << std::endl;
    }

    void drawCommandResults() {
        if (commandResults.empty()) return;

        std::string primary = getColorCode(config.primaryColor);
        std::string secondary = getColorCode(config.secondaryColor);

        std::cout << primary << "\n╔ КОМАНДЫ (↑/↓ навигация, ENTER выполнить) ╗" << RESET << std::endl;
        for (size_t i = 0; i < commandResults.size(); ++i) {
            if (static_cast<int>(i) == selectedCommand) {
                std::cout << secondary << "► " << BRIGHT_CYAN << BOLD
                         << commandLabels[i] << ": " << RESET << BRIGHT_GREEN
                         << commandResults[i] << RESET << std::endl;
            } else {
                std::cout << primary << "  " << DIM << commandLabels[i] << ": "
                         << BRIGHT_CYAN << commandResults[i] << RESET << std::endl;
            }
        }
    }

public:
    CyberpunkInterface() : config{"cyan", "magenta", 50, true, true, "simple", {}} {}

    void initialize(const std::string& configFile) {
        loadConfig(configFile);
        enableRawMode();
        int ret = system("clear");
        (void)ret;
        executeAllCommands();
    }

    void shutdown() {
        disableRawMode();
        clearScreen();
        std::cout << RESET;
    }

    void render() {
        clearScreen();
        drawBorder();
        drawLogo();
        std::cout << std::endl;
        drawStatusBar();
        std::cout << std::endl;

        if (config.enableAnimation) {
            drawDataStream();
        }

        std::cout << std::endl;
        drawCommandResults();
        std::cout << std::endl;
        drawKeyInfo();
        drawMenu();

        frameCount++;
    }

    void processInput() {
        char key = readKey();
        if (key != '\0') {
            if (key == '\033') {  // Escape sequence for arrow keys
                char next1 = readKey();
                if (next1 == '[') {
                    char next2 = readKey();
                    if (next2 == 'A') {  // Up arrow - циклическая навигация
                        selectedCommand--;
                        if (selectedCommand < 0) {
                            selectedCommand = config.commands.size() - 1;
                        }
                        return;
                    } else if (next2 == 'B') {  // Down arrow - циклическая навигация
                        selectedCommand++;
                        if (selectedCommand >= static_cast<int>(config.commands.size())) {
                            selectedCommand = 0;
                        }
                        return;
                    }
                }
            }

            switch (key) {
                case 'q':
                case 'Q':
                    running = false;
                    break;
                case 'c':
                case 'C':
                    keyPresses.clear();
                    break;
                case 'r':
                case 'R':
                    frameCount = 0;
                    keyPresses.clear();
                    break;
                case '\n':  // Enter key - выполнить команду и выйти
                case '\r':
                    executeSelectedCommand();
                    outputAndExit();
                    break;
                default:
                    keyPresses[key]++;
                    break;
            }
        }
    }

    void printFinalResult() {
        if (selectedCommand >= 0 && selectedCommand < static_cast<int>(commandResults.size())) {
            std::string primary = getColorCode(config.primaryColor);
            std::string secondary = getColorCode(config.secondaryColor);

            clearScreen();
            std::cout << primary << BOLD << "\n╔════════════════════════════════════════╗" << RESET << std::endl;
            std::cout << secondary << BOLD << "║     РЕЗУЛЬТАТ ВЫПОЛНЕНИЯ КОМАНДЫ       ║" << RESET << std::endl;
            std::cout << primary << "╠════════════════════════════════════════╣" << RESET << std::endl;
            std::cout << primary << "║ Команда " << (selectedCommand + 1) << ": " << RESET << std::endl;
            std::cout << primary << "║   " << BRIGHT_CYAN << config.commands[selectedCommand].substr(0, 35) << RESET << std::endl;
            std::cout << primary << "╠════════════════════════════════════════╣" << RESET << std::endl;
            std::cout << secondary << BOLD << "║ ВЫВОД:" << RESET << std::endl;
            std::cout << primary << "║  " << BRIGHT_GREEN << commandResults[selectedCommand] << RESET << std::endl;
            std::cout << primary << "╚════════════════════════════════════════╝" << RESET << std::endl;
            std::cout << "\n";
        }
    }

    void outputAndExit() {
        running = false;
    }

    void run() {
        while (running) {
            processInput();
            render();

            int delayMs = 1000 / config.updateRate;
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        }
    }

    bool isRunning() const {
        return running;
    }
};

int main() {
    CyberpunkInterface interface;

    try {
        interface.initialize("cyberpunk.cfg");
        interface.run();
        interface.printFinalResult();
        interface.shutdown();

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        interface.shutdown();
        return 1;
    }

    return 0;
}
