// src/AI/hunter/hunter.h
#ifndef HUNTER_H
#define HUNTER_H

#include <memory>

struct HunterTask;

// Heartbeat entry
void hunter_tick();

// Task enqueuer
void hunter_enqueue(std::shared_ptr<HunterTask> task);

#endif // HUNTER_H
