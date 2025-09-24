#ifndef LNR_STATE_H
#define LNR_STATE_H

// Lanner state machine
enum LannerState {
    LNR_STATE_IDLE = 0,   // Default state
    LNR_STATE_CASTING,    // Currently casting a skill
    LNR_STATE_BUFFING     // Maintaining self-buffs
};

#endif // LNR_STATE_H
