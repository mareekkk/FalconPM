// src/AI/hunter/hnt_state.h
#ifndef HNT_STATE_H
#define HNT_STATE_H

enum class HunterState {
    IDLE,
    QUEUED,
    RUNNING,
    COOLDOWN
};

// Single public entry called by the global heartbeat
void hunter_tick();

#endif // HNT_STATE_H
