// src/AI/hunter/hnt_api.h
#ifndef HNT_API_H
#define HNT_API_H

#include "../infra/plugin_api.h"   // HunterAPI struct is defined here

// Global symbol (defined in hnt_api.cpp)
extern HunterAPI g_hunter_api;

// Init function (fills function pointers)
void hnt_api_init();

#endif // HNT_API_H
