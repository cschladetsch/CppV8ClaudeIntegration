#include "ClaudeConsole.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <format> 
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <cctype>

namespace fs = std::filesystem;

namespace claude_console {

ClaudeConsole::ClaudeConsole()
    : mode_(ConsoleMode::Shell) {
    
    // Create config directory if it doesn't exist
    CreateConfigDirectory();
    
    // Load user configuration
    LoadConfiguration();
    
    // Initialize built-in commands
    builtinCommands_ = {
        {"help", "Show help message"},
        {"quit", "Exit the console"},
        {"exit", "Exit the console"},
        {"clear", "Clear the console"},
        {"js", "Switch to JavaScript mode"},
        {"javascript", "Switch to JavaScript mode"},
        {"shell", "Switch to shell mode"},
        {"sh", "Switch to shell mode"},
        {"ask", "Ask Claude AI a question"},
        {"config", "Manage configuration and aliases"},
        {"reload", "Reload configuration from files"}
    };
}

ClaudeConsole::~ClaudeConsole() {
    Shutdown();
}

bool ClaudeConsole::Initialize() {
    // For this simplified version, just return true
    // In a full implementation, this would initialize V8
    return true;
}

void ClaudeConsole::Shutdown() {
    // Cleanup resources
}

CommandResult ClaudeConsole::ExecuteCommand(const std::string& command) {
    if (command.empty()) {
        return {true, "", "", std::chrono::microseconds(0), 0};
    }
    
    // Check for mode switch commands
    if (command == "js" || command == "javascript") {
        SetMode(ConsoleMode::JavaScript);
        return {true, "Switched to JavaScript mode", "", std::chrono::microseconds(0), 0};
    } else if (command == "shell" || command == "sh") {
        SetMode(ConsoleMode::Shell);
        return {true, "Switched to Shell mode", "", std::chrono::microseconds(0), 0};
    }
    
    // Handle built-in commands
    if (IsBuiltinCommand(command)) {
        return ExecuteBuiltinCommand(command);
    }
    
    // Execute based on mode
    if (mode_ == ConsoleMode::JavaScript) {
        return ExecuteJavaScript(command);
    } else {
        // In shell mode, check for special prefixes
        if (!command.empty() && command[0] == '&') {
            // & prefix for JavaScript
            return ExecuteJavaScript(command.substr(1));
        } else if (!command.empty() && command[0] == '?') {
            // ? prefix for Claude AI queries
            return ExecuteClaudeQuery(command.substr(1));
        }
        return ExecuteShellCommand(command);
    }
}

CommandResult ClaudeConsole::ExecuteJavaScript(const std::string& code) {
    // For this demo, we'll just simulate JavaScript execution
    // In a full implementation, this would use V8
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    CommandResult result;
    result.success = true;
    result.output = std::format("// JavaScript execution simulated\n// Code: {}", code);
    result.executionTime = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - startTime);
    result.exitCode = 0;
    
    return result;
}

CommandResult ClaudeConsole::ExecuteShellCommand(const std::string& command) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Execute shell command
    FILE* pipe = popen((command + " 2>&1").c_str(), "r");
    if (!pipe) {
        return {false, "", "Failed to execute command", std::chrono::microseconds(0), 127};
    }
    
    std::string output;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        output += buffer;
    }
    
    int exitCode = pclose(pipe);
    auto endTime = std::chrono::high_resolution_clock::now();
    
    CommandResult result;
    result.success = (WEXITSTATUS(exitCode) == 0);
    result.output = output;
    result.exitCode = WEXITSTATUS(exitCode);
    result.executionTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    return result;
}

CommandResult ClaudeConsole::ExecuteClaudeQuery(const std::string& question) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    CommandResult result;
    result.success = true;
    result.exitCode = 0;
    
    // Built-in knowledge base for common questions
    std::string lowerQuestion = question;
    std::transform(lowerQuestion.begin(), lowerQuestion.end(), lowerQuestion.begin(), ::tolower);
    
    if (lowerQuestion.find("capital") != std::string::npos && lowerQuestion.find("canada") != std::string::npos) {
        result.output = "Ottawa";
    } else if (lowerQuestion.find("capital") != std::string::npos && lowerQuestion.find("france") != std::string::npos) {
        result.output = "Paris";
    } else if (lowerQuestion.find("capital") != std::string::npos && lowerQuestion.find("japan") != std::string::npos) {
        result.output = "Tokyo";
    } else if (lowerQuestion.find("capital") != std::string::npos && lowerQuestion.find("usa") != std::string::npos || 
               lowerQuestion.find("united states") != std::string::npos) {
        result.output = "Washington, D.C.";
    } else if (lowerQuestion.find("capital") != std::string::npos && lowerQuestion.find("uk") != std::string::npos ||
               lowerQuestion.find("united kingdom") != std::string::npos) {
        result.output = "London";
    } else if (lowerQuestion.find("time") != std::string::npos || lowerQuestion.find("date") != std::string::npos) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        result.output = std::ctime(&time_t);
        // Remove trailing newline
        if (!result.output.empty() && result.output.back() == '\n') {
            result.output.pop_back();
        }
    } else if (lowerQuestion.find("hello") != std::string::npos || lowerQuestion.find("hi") != std::string::npos) {
        result.output = "Hello! How can I help you today?";
    } else if (lowerQuestion.find("help") != std::string::npos) {
        result.output = "I can help answer basic questions about:\n";
        result.output += "- World capitals (e.g., 'what is capital of canada')\n";
        result.output += "- Current time and date\n";
        result.output += "- Basic greetings\n";
        result.output += "Type 'help' for console commands or try asking me something!";
    } else {
        // Try to find PyClaudeCli or 'ask' command as fallback
        FILE* checkPipe = popen("which ask 2>/dev/null", "r");
        if (checkPipe) {
            char buffer[256];
            bool hasAsk = (fgets(buffer, sizeof(buffer), checkPipe) != nullptr);
            pclose(checkPipe);
            
            if (hasAsk) {
                // Execute ask command with the question
                std::string askCommand = "ask \"" + question + "\" 2>&1";
                return ExecuteSubprocess(askCommand);
            }
        }
        
        // Default response for unknown questions
        result.output = "I don't have built-in knowledge about that topic. ";
        result.output += "I can answer questions about world capitals, time, and basic greetings. ";
        result.output += "For more advanced questions, please install Claude CLI or add 'ask' to your PATH.";
    }
    
    result.executionTime = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - startTime);
    
    return result;
}

CommandResult ClaudeConsole::ExecuteSubprocess(const std::string& command) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return {false, "", "Failed to execute command", std::chrono::microseconds(0), 1};
    }
    
    std::string output;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        output += buffer;
    }
    
    int exitCode = pclose(pipe);
    auto endTime = std::chrono::high_resolution_clock::now();
    
    CommandResult result;
    result.success = (WEXITSTATUS(exitCode) == 0);
    result.output = output;
    result.exitCode = WEXITSTATUS(exitCode);
    result.executionTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    if (!result.success && !output.empty()) {
        result.error = output;
        result.output = "";
    }
    
    return result;
}

bool ClaudeConsole::IsBuiltinCommand(const std::string& command) const {
    auto words = SplitCommand(command);
    if (words.empty()) return false;
    
    return builtinCommands_.find(words[0]) != builtinCommands_.end();
}

CommandResult ClaudeConsole::ExecuteBuiltinCommand(const std::string& command) {
    auto words = SplitCommand(command);
    if (words.empty()) {
        return {false, "", "Empty command", std::chrono::microseconds(0), 1};
    }
    
    const std::string& cmd = words[0];
    CommandResult result;
    result.success = true;
    result.exitCode = 0;
    
    if (cmd == "help") {
        result.output = "Available commands:\n";
        for (const auto& [name, desc] : builtinCommands_) {
            result.output += std::format("  {} - {}\n", name, desc);
        }
        result.output += "\nSpecial features:\n";
        result.output += "  &<javascript> - Execute JavaScript from shell mode (e.g., &Math.sqrt(16))\n";
        result.output += "  ?<question> - Ask Claude AI a question (e.g., ?what is capital of canada)\n";
        result.output += "\nCurrent mode: " + std::string(mode_ == ConsoleMode::JavaScript ? "JavaScript" : "Shell");
    } else if (cmd == "quit" || cmd == "exit") {
        result.output = "Exiting...";
        // The UI layer should handle actual exit
    } else if (cmd == "clear") {
        result.output = "\033[2J\033[H"; // ANSI clear screen
    } else if (cmd == "ask") {
        // Execute Claude query
        if (words.size() > 1) {
            // Join all words after "ask" to form the question
            std::string question;
            for (size_t i = 1; i < words.size(); ++i) {
                if (i > 1) question += " ";
                question += words[i];
            }
            
            return ExecuteClaudeQuery(question);
        } else {
            result.success = false;
            result.error = "Usage: ask <question>";
            result.exitCode = 1;
        }
    } else if (cmd == "config") {
        if (words.size() == 1) {
            // Show config directory
            result.output = "Configuration directory: " + GetConfigPath() + "\n";
            result.output += "Configuration files:\n";
            result.output += "  config.json - Main configuration\n";
            result.output += "  aliases - Command aliases\n";
            result.output += "\nUse 'reload' to reload configuration from files\n";
        } else if (words.size() >= 3 && words[1] == "alias") {
            // Set alias: config alias name=value
            std::string aliasCmd = command.substr(command.find("alias") + 6); // Skip "config alias "
            size_t eq = aliasCmd.find('=');
            if (eq != std::string::npos) {
                std::string name = aliasCmd.substr(0, eq);
                std::string value = aliasCmd.substr(eq + 1);
                // Remove quotes if present
                if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.size() - 2);
                }
                SetAlias(name, value);
                SaveConfiguration();
                result.output = std::format("Alias set: {} = '{}'", name, value);
            } else {
                result.success = false;
                result.error = "Usage: config alias name=value";
                result.exitCode = 1;
            }
        } else {
            result.success = false;
            result.error = "Usage: config [alias name=value]";
            result.exitCode = 1;
        }
    } else if (cmd == "reload") {
        LoadConfiguration();
        result.output = "Configuration reloaded from " + GetConfigPath();
    } else {
        result.success = false;
        result.error = "Unknown command: " + cmd;
        result.exitCode = 1;
    }
    
    return result;
}

std::string ClaudeConsole::FormatExecutionTime(const std::chrono::microseconds& us) {
    if (us.count() < 1000) {
        return std::format("{}μs", us.count());
    } else if (us.count() < 1000000) {
        return std::format("{:.1f}ms", us.count() / 1000.0);
    } else {
        return std::format("{:.2f}s", us.count() / 1000000.0);
    }
}

std::vector<std::string> ClaudeConsole::SplitCommand(const std::string& command) {
    std::vector<std::string> words;
    std::istringstream iss(command);
    std::string word;
    
    while (iss >> word) {
        words.push_back(word);
    }
    
    return words;
}

std::string ClaudeConsole::FindPyClaudeCliPath() {
    // Check common locations for PyClaudeCli
    std::vector<std::string> searchPaths = {
        "../PyClaudeCli/main.py",
        "../../PyClaudeCli/main.py",
        "../../../PyClaudeCli/main.py",
        fs::path(fs::current_path()).parent_path() / "PyClaudeCli" / "main.py"
    };
    
    for (const auto& path : searchPaths) {
        if (fs::exists(path)) {
            return fs::absolute(path).string();
        }
    }
    
    return "";
}

void ClaudeConsole::CreateConfigDirectory() {
    std::string configDir = GetConfigPath();
    if (!fs::exists(configDir)) {
        fs::create_directories(configDir);
        
        // Create default configuration files
        std::string configFile = configDir + "/config.json";
        if (!fs::exists(configFile)) {
            std::ofstream config(configFile);
            config << "{\n";
            config << "  \"default_mode\": \"shell\",\n";
            config << "  \"prompt_format\": \"[{mode}] λ \",\n";
            config << "  \"show_execution_time\": true,\n";
            config << "  \"history_size\": 1000,\n";
            config << "  \"enable_colors\": true,\n";
            config << "  \"claude_integration\": {\n";
            config << "    \"enabled\": true,\n";
            config << "    \"timeout_seconds\": 30\n";
            config << "  },\n";
            config << "  \"aliases\": {\n";
            config << "    \"ll\": \"ls -la\",\n";
            config << "    \"la\": \"ls -la\",\n";
            config << "    \"...\": \"cd ../..\"\n";
            config << "  }\n";
            config << "}\n";
            config.close();
        }
        
        // Create aliases file
        std::string aliasFile = configDir + "/aliases";
        if (!fs::exists(aliasFile)) {
            std::ofstream aliases(aliasFile);
            aliases << "# Claude Console Aliases\n";
            aliases << "# Format: alias_name=command\n";
            aliases << "ll=ls -la\n";
            aliases << "la=ls -la\n";
            aliases << "...=cd ../..\n";
            aliases << "cls=clear\n";
            aliases << "q=quit\n";
            aliases.close();
        }
    }
}

void ClaudeConsole::LoadConfiguration() {
    // First load shared configuration
    LoadSharedConfiguration();
    
    // Then load app-specific configuration
    std::string configFile = GetConfigPath() + "/config.json";
    if (fs::exists(configFile)) {
        // TODO: Parse JSON configuration
        // For now, just load basic aliases from aliases file
        std::string aliasFile = GetConfigPath() + "/aliases";
        if (fs::exists(aliasFile)) {
            std::ifstream file(aliasFile);
            std::string line;
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;
                
                size_t eq = line.find('=');
                if (eq != std::string::npos) {
                    std::string name = line.substr(0, eq);
                    std::string value = line.substr(eq + 1);
                    SetAlias(name, value);
                }
            }
        }
    }
}

std::string ClaudeConsole::GetSharedConfigPath() const {
    const char* home = std::getenv("HOME");
    if (!home) {
        home = std::getenv("USERPROFILE"); // Windows fallback
    }
    
    if (home) {
        return std::string(home) + "/.config/shared";
    }
    
    return "./.config/shared"; // Fallback to current directory
}

void ClaudeConsole::LoadSharedConfiguration() {
    std::string sharedAliasFile = GetSharedConfigPath() + "/aliases";
    if (fs::exists(sharedAliasFile)) {
        std::ifstream file(sharedAliasFile);
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                std::string name = line.substr(0, eq);
                std::string value = line.substr(eq + 1);
                SetAlias(name, value);
            }
        }
    }
}

void ClaudeConsole::SaveConfiguration() {
    // TODO: Save current configuration to JSON
    // For now, just save aliases
    std::string aliasFile = GetConfigPath() + "/aliases";
    std::ofstream file(aliasFile);
    file << "# Claude Console Aliases\n";
    file << "# Format: alias_name=command\n";
    for (const auto& [name, value] : aliases_) {
        file << name << "=" << value << "\n";
    }
}

std::string ClaudeConsole::GetConfigPath() const {
    const char* home = std::getenv("HOME");
    if (!home) {
        home = std::getenv("USERPROFILE"); // Windows fallback
    }
    
    if (home) {
        return std::string(home) + "/.config/cll";
    }
    
    return "./.config/cll"; // Fallback to current directory
}

void ClaudeConsole::SetAlias(const std::string& name, const std::string& value) {
    aliases_[name] = value;
}

std::string ClaudeConsole::ExpandAlias(const std::string& command) const {
    auto words = SplitCommand(command);
    if (words.empty()) return command;
    
    auto it = aliases_.find(words[0]);
    if (it != aliases_.end()) {
        std::string expanded = it->second;
        for (size_t i = 1; i < words.size(); ++i) {
            expanded += " " + words[i];
        }
        return expanded;
    }
    
    return command;
}

void ClaudeConsole::Output(const std::string& text) {
    if (outputCallback_) {
        outputCallback_(text);
    } else {
        std::cout << text;
    }
}

void ClaudeConsole::Error(const std::string& text) {
    if (errorCallback_) {
        errorCallback_(text);
    } else {
        std::cerr << text;
    }
}

// CommandHistory implementation
CommandHistory::CommandHistory(size_t maxSize) 
    : maxSize_(maxSize), position_(-1) {
}

void CommandHistory::Add(const std::string& command) {
    if (command.empty()) return;
    
    // Don't add duplicates of the last command
    if (!history_.empty() && history_.back() == command) {
        return;
    }
    
    history_.push_back(command);
    
    // Maintain max size
    if (history_.size() > maxSize_) {
        history_.erase(history_.begin());
    }
    
    ResetPosition();
}

const std::string& CommandHistory::Get(size_t index) const {
    static const std::string empty;
    if (index >= history_.size()) {
        return empty;
    }
    return history_[index];
}

std::string CommandHistory::GetPrevious() {
    if (history_.empty()) return "";
    
    if (position_ == -1) {
        position_ = static_cast<int>(history_.size()) - 1;
    } else if (position_ > 0) {
        position_--;
    }
    
    return position_ >= 0 ? history_[position_] : "";
}

std::string CommandHistory::GetNext() {
    if (history_.empty() || position_ == -1) return "";
    
    if (position_ < static_cast<int>(history_.size()) - 1) {
        position_++;
        return history_[position_];
    } else {
        position_ = -1;
        return "";
    }
}

} // namespace claude_console