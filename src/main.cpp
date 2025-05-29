#include "core/logger.hpp"

signed main() {
    Logger::init();  // По умолчанию: vela.log
    Logger::set_level(LogLevel::DEBUG);  // Показывать все сообщения

    Logger::info("Starting Vela...");
    Logger::debug("Loaded config: 5 modules");
    Logger::warning("Hyprland IPC not connected!");
    Logger::error("Failed to load style.css");

    return 0;
}