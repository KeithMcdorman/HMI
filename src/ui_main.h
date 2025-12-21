#pragma once

#include "lvgl.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------- Public UI API ----------
void ui_build_live_view(lv_obj_t * parent);

// NOTE: Keep this signature EXACTLY matching what your main.cpp calls.
// If your main.cpp call differs, adjust this prototype to match main.cpp.
void ui_update_live_values(
    float iso_hp_psi,      float resin_hp_psi,
    float iso_low_psi,     float resin_low_psi,
    float primary_air_psi, float gun_air_psi,
    float iso_hp_temp_f,   float resin_hp_temp_f,
    float iso_low_temp_f,  float resin_low_temp_f,
    float hose1_temp_f,    float hose2_temp_f,
    float ratio
);

// Banner
void ui_set_banner(const char * msg, bool is_error);

// Hose heat callbacks (UI -> your application logic)
typedef void (*ui_hose_toggle_cb_t)(uint8_t zone, bool enabled);
typedef void (*ui_hose_setpoint_cb_t)(uint8_t zone, int setpoint_f);

void ui_set_hose_toggle_callback(ui_hose_toggle_cb_t cb);
void ui_set_hose_setpoint_callback(ui_hose_setpoint_cb_t cb);

#ifdef __cplusplus
} // extern "C"
#endif
