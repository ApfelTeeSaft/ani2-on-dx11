//
//	defines.h - Modern Windows 11 port
//
///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2001, Pipeworks Software Inc. (Original)
//  Modern Port Copyright (c) 2023
//				All rights reserved
#pragma once

#include <DirectXMath.h>

// Comment this out in the final build
//#define DEBUG_BUILD

// Animation timing constants
constexpr float DEMO_TOTAL_TIME = 8.0f;
constexpr float FINAL_HOLD_TIME = 2.0f;
constexpr float FINISH_TRANSITION_TIME = 0.8f;

// Text animation constants
constexpr float TEXT_ANIM_START_TIME = (DEMO_TOTAL_TIME - FINAL_HOLD_TIME);
constexpr float TEXT_ANIM_LEN = 0.25f;

// Blob animation constants
constexpr float BLOB_STATIC_END_TIME = 0.6f;
constexpr float OO_BLOB_STATIC_END_TIME = (1.0f / BLOB_STATIC_END_TIME);
constexpr float BLOB_ZERO_INTENSE_END_TIME = (BLOB_STATIC_END_TIME + 0.5f);
constexpr float BLOB_BASE_INTENSITY = 0.3f;

// Finish animation constants
constexpr float FINISH_START_TIME = (DEMO_TOTAL_TIME - FINAL_HOLD_TIME - FINISH_TRANSITION_TIME);
constexpr float FINISH_STOP_TIME = (DEMO_TOTAL_TIME - FINAL_HOLD_TIME);
constexpr float OO_FINISH_DELTA = (1.0f / FINISH_TRANSITION_TIME);

// Intensity animation constants
constexpr float MAX_INTENSITY_TIME = (FINISH_START_TIME - 0.0f);
constexpr float MAX_INTENSITY_DELTA = (MAX_INTENSITY_TIME - BLOB_ZERO_INTENSE_END_TIME);
constexpr float OO_MAX_INTENSITY_DELTA = (1.0f / MAX_INTENSITY_DELTA);
constexpr float DEMO_START_INTENSITY = 0.0f;

// Blob pulse animation constants
constexpr float BLOB_PULSE_START = (BLOB_STATIC_END_TIME);
constexpr float BLOB_PULSE_END = (FINISH_STOP_TIME - 0.4f);
constexpr float BLOB_PULSE_ELAPSED = (BLOB_PULSE_END - BLOB_PULSE_START);

// Blob jitter animation constants
constexpr float BLOB_JITTER_START = (BLOB_STATIC_END_TIME);
constexpr float BLOB_JITTER_DELTA = (FINISH_START_TIME);
constexpr float OO_BLOB_JITTER_DELTA = (1.0f / BLOB_JITTER_DELTA);

// Scene animation constants
constexpr float SCENE_ANIM_LEN = 4.5f;
constexpr float SCENE_ANIM_START_TIME = (BLOB_STATIC_END_TIME + 0.25f);

// Push-out animation constants
constexpr float START_PUSHOUT_RADIUS = 0.0f;
constexpr float PUSHOUT_START_TIME = 0.5f;
constexpr float PUSHOUT_DELTA = 2.7f;
constexpr float OO_PUSHOUT_DELTA = (1.0f / PUSHOUT_DELTA);

// Shield animation constants
constexpr float SHIELD_FADE_IN_START_TIME = (BLOB_STATIC_END_TIME);
constexpr float SHIELD_FADE_IN_DELTA = 1.2f;
constexpr float OO_SHIELD_FADE_IN_DELTA = (1.0f / SHIELD_FADE_IN_DELTA);
constexpr float SHIELD_FADE_OUT_START_TIME = (FINISH_START_TIME - 0.1f);
constexpr float SHIELD_FADE_OUT_DELTA = (FINISH_TRANSITION_TIME * 0.2f);
constexpr float OO_SHIELD_FADE_OUT_DELTA = (1.0f / SHIELD_FADE_OUT_DELTA);

// Glow animation constants
constexpr float GLOW_FADE_CIRCLE_START = (FINISH_START_TIME - 0.5f);
constexpr float GLOW_FADE_CIRCLE_MUL = (1.0f / 0.3f);
constexpr float GLOW_FADE_SCREEN_START = (GLOW_FADE_CIRCLE_START + 0.3f);
constexpr float GLOW_FADE_SCREEN_MUL = (1.0f / 0.25f);

// Slash gradient animation constants
constexpr float SLASH_GRADIENT_TRANSITION_START = (FINISH_START_TIME - 0.5f);
constexpr float SLASH_GRADIENT_TRANSITION_END = (FINISH_STOP_TIME);
constexpr float SLASH_GRADIENT_TRANSITION_MUL = (1.0f / (SLASH_GRADIENT_TRANSITION_END - SLASH_GRADIENT_TRANSITION_START));

// Scene detail settings
constexpr float SCENE_LO_DETAIL_START = (FINISH_START_TIME);

// Feature flags
#ifdef DEBUG_BUILD
#define INCLUDE_PLACEMENT_DOODAD
#define INCLUDE_INPUT
#endif