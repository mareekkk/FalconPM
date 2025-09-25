// src/AI/hunter/hnt_task.cpp
#include "hnt_task.h"
#include <iostream>

#define COLOR_RESET  "\033[0m"
#define COLOR_HUNTER "\033[36m"

bool BuffTask::execute() {
    std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
              << " Executing BuffTask (skill=" << skill_id
              << " target=" << target_id << ")" << std::endl;
    return true; // placeholder
}

bool AttackTask::execute() {
    std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
              << " Executing AttackTask (target=" << target_id << ")" << std::endl;
    return true;
}

bool MoveTask::execute() {
    std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
              << " Executing MoveTask (" << map
              << " " << x << "," << y << ")" << std::endl;
    return true;
}

bool HealTask::execute() {
    std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
              << " Executing HealTask (item=" << item_id
              << " amount=" << amount << ")" << std::endl;
    return true;
}
