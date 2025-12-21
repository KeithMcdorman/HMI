#pragma once
#include "lvgl.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Build the live-only screen
void ui_build_live_view(lv_obj_t *root);

// Update all live values in one call (matches your main.cpp usage)
void ui_update_live_values(
    float iso_hp_psi,      float resin_hp_psi,
    float iso_low_psi,     float resin_low_psi,
    float primary_air_psi, float gun_air_psi,
    float iso_hp_temp_f,   float resin_hp_temp_f,
    float iso_low_temp_f,  float resin_low_temp_f,
    float hose1_temp_f,    float hose2_temp_f,
    float ratio
);

// Optional: set banner message and severity (error=true shows red styling)
void ui_set_banner(const char *msg, bool error);

// Optional: callback hook when user toggles hose heat or changes setpoint
typedef void (*ui_hose_toggle_cb_t)(int hose_index, bool enabled);
typedef void (*ui_hose_setpoint_cb_t)(int hose_index, int setpoint_f);

void ui_set_hose_toggle_callback(ui_hose_toggle_cb_t cb);
void ui_set_hose_setpoint_callback(ui_hose_setpoint_cb_t cb);

#ifdef __cplusplus
} // extern "C"
#endif
