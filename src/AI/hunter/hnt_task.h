// src/AI/hunter/hnt_task.h
#ifndef HNT_TASK_H
#define HNT_TASK_H

#include <string>
#include <memory>
#include <cstdint>

// Task categories
enum class HunterTaskType {
    BUFF,
    ATTACK,
    MOVE,
    HEAL
};

// Base Task
struct HunterTask {
    HunterTaskType type;
    uint64_t enqueue_tick;
    uint64_t cooldown_ms;
    int priority;

    HunterTask(HunterTaskType t, int prio = 0, uint64_t cd = 0)
        : type(t), enqueue_tick(0), cooldown_ms(cd), priority(prio) {}

    virtual ~HunterTask() = default;

    virtual std::string name() const = 0;
    virtual bool execute() = 0;
};

// Buff Task
struct BuffTask : public HunterTask {
    int skill_id;
    int target_id;

    BuffTask(int skill, int target)
        : HunterTask(HunterTaskType::BUFF, 10, 500),
          skill_id(skill), target_id(target) {}

    std::string name() const override { return "BuffTask"; }
    bool execute() override;
};

// Attack Task
struct AttackTask : public HunterTask {
    int target_id;

    AttackTask(int target)
        : HunterTask(HunterTaskType::ATTACK, 5, 250),
          target_id(target) {}

    std::string name() const override { return "AttackTask"; }
    bool execute() override;
};

// Move Task
struct MoveTask : public HunterTask {
    std::string map;
    int x, y;

    MoveTask(std::string m, int px, int py)
        : HunterTask(HunterTaskType::MOVE, 3, 200),
          map(std::move(m)), x(px), y(py) {}

    std::string name() const override { return "MoveTask"; }
    bool execute() override;
};

// Heal Task
struct HealTask : public HunterTask {
    int item_id;
    int amount;

    HealTask(int item, int amt)
        : HunterTask(HunterTaskType::HEAL, 8, 400),
          item_id(item), amount(amt) {}

    std::string name() const override { return "HealTask"; }
    bool execute() override;
};

#endif // HNT_TASK_H
