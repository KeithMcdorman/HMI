#include "ui_main.h"

#include <math.h>
#include <cstring>  // strstr
#include <cstdio>   // snprintf

// ----------------------------------
// Internal state / handles
// ----------------------------------
typedef struct {
    // Banner
    lv_obj_t *banner;
    lv_obj_t *banner_lbl;
    bool      banner_is_error;

    // HP ISO
    lv_obj_t *arc_iso_hp;
    lv_obj_t *lbl_iso_hp_psi;
    lv_obj_t *lbl_iso_hp_temp;
    lv_obj_t *lbl_iso_hp_lowtemp;

    // HP RESIN
    lv_obj_t *arc_resin_hp;
    lv_obj_t *lbl_resin_hp_psi;
    lv_obj_t *lbl_resin_hp_temp;
    lv_obj_t *lbl_resin_hp_lowtemp;

    // Ratio
    lv_obj_t *ratio_bar_bg;
    lv_obj_t *ratio_bar_fill;
    lv_obj_t *lbl_ratio;

    // Controls
    lv_obj_t *btn_spray;
    lv_obj_t *btn_drum_air;

    // Hose heat (center)
    lv_obj_t *lbl_h1_status;
    lv_obj_t *lbl_h1_temp;
    lv_obj_t *lbl_h1_set;
    lv_obj_t *btn_h1_toggle;
    lv_obj_t *btn_h1_up;
    lv_obj_t *btn_h1_down;

    lv_obj_t *lbl_h2_status;
    lv_obj_t *lbl_h2_temp;
    lv_obj_t *lbl_h2_set;
    lv_obj_t *btn_h2_toggle;
    lv_obj_t *btn_h2_up;
    lv_obj_t *btn_h2_down;

    // Bottom row
    lv_obj_t *arc_iso_low;
    lv_obj_t *lbl_iso_low_psi;
    lv_obj_t *lbl_iso_low_temp;

    lv_obj_t *arc_resin_low;
    lv_obj_t *lbl_resin_low_psi;
    lv_obj_t *lbl_resin_low_temp;

    lv_obj_t *arc_primary_air;
    lv_obj_t *lbl_primary_air_psi;

    lv_obj_t *arc_gun_air;
    lv_obj_t *lbl_gun_air_psi;

    // Center column cards
    lv_obj_t *lbl_sys_status;
    lv_obj_t *lbl_interlock;
    lv_obj_t *lbl_status_card;
    lv_obj_t *btn_estop;
    lv_obj_t *lbl_estop;

    // Internal values
    int  hose1_set_f;
    int  hose2_set_f;
    bool hose1_on;
    bool hose2_on;

} ui_live_t;

static ui_live_t g = {0};

static ui_hose_toggle_cb_t   s_hose_toggle_cb   = nullptr;
static ui_hose_setpoint_cb_t s_hose_setpoint_cb = nullptr;

// ----------------------------------
// Helpers
// ----------------------------------

// Forward declaration (ui_set_banner is defined before banner_apply_style)
static void banner_apply_style(bool error);

extern "C" void ui_set_banner(const char* msg, bool is_error)
{
    // If your UI hasn't been built yet, just ignore.
    if(!g.banner) return;

    // Treat nullptr / empty as "hide banner"
    const bool hide = (msg == nullptr) || (msg[0] == '\0');

    if(hide) {
        lv_obj_add_flag(g.banner, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    // Apply style and show
    banner_apply_style(is_error);

    if(g.banner_lbl) lv_label_set_text(g.banner_lbl, msg);
    if(g.lbl_status_card) lv_label_set_text(g.lbl_status_card, msg);

    lv_obj_clear_flag(g.banner, LV_OBJ_FLAG_HIDDEN);
}


static inline float clampf(float v, float lo, float hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

static void noscroll(lv_obj_t *o)
{
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(o, LV_SCROLLBAR_MODE_OFF);
}

static void transparent_container(lv_obj_t *o)
{
    noscroll(o);
    lv_obj_set_style_bg_opa(o, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
}

static void style_card(lv_obj_t *o)
{
    noscroll(o);
    lv_obj_set_style_radius(o, 14, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_30, 0);
    lv_obj_set_style_bg_color(o, lv_color_hex(0x0B1220), 0);
    lv_obj_set_style_border_width(o, 2, 0);
    lv_obj_set_style_border_opa(o, LV_OPA_50, 0);
    lv_obj_set_style_border_color(o, lv_color_hex(0x6B7A99), 0);
    lv_obj_set_style_pad_all(o, 12, 0);
}


static lv_obj_t* card_begin(lv_obj_t *parent)
{
    // Treat the provided object as the card itself (no nested "card inside card").
    style_card(parent);
    noscroll(parent);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    return parent;
}

static void style_title(lv_obj_t *lbl)
{
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(lbl, LV_OPA_90, 0);
    lv_obj_set_width(lbl, lv_pct(100));
}

static void style_value_big(lv_obj_t *lbl)
{
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(lbl, LV_OPA_100, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_48, 0);
    lv_obj_set_width(lbl, lv_pct(100));
}

static void style_value_med(lv_obj_t *lbl)
{
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(lbl, LV_OPA_100, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_32, 0);
    lv_obj_set_width(lbl, lv_pct(100));
}

static void style_value_small(lv_obj_t *lbl)
{
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(lbl, LV_OPA_100, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_24, 0);
    lv_obj_set_width(lbl, lv_pct(100));
}

static void arc_make_visual_only(lv_obj_t *arc)
{
    // prevent “slider” behavior
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    // hide knob completely
    lv_obj_set_style_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_pad_all(arc, 0, LV_PART_KNOB);

    // no focus state visuals
    lv_obj_clear_state(arc, LV_STATE_FOCUSED);
    lv_obj_clear_state(arc, LV_STATE_PRESSED);
}

static void hose_set_label(lv_obj_t *lbl_set, int set_f)
{
    if(!lbl_set) return;
    char b[32];
    snprintf(b, sizeof(b), "Set: %d \xC2\xB0""F", set_f);
    lv_label_set_text(lbl_set, b);
}

static void hose_toggle_button_text(lv_obj_t *btn, bool on)
{
    if(!btn) return;
    lv_obj_t *lbl = lv_obj_get_child(btn, 0);
    if(!lbl) return;
    // Keep this concise; the card already says "HOSE X HEAT".
    lv_label_set_text(lbl, on ? "ON" : "OFF");
}

// ----------------------------------
// Banner
// ----------------------------------
static void banner_apply_style(bool error)
{
    if(!g.banner) return;
    g.banner_is_error = error;

    if(error) {
        lv_obj_set_style_bg_color(g.banner, lv_color_hex(0x3A0D0D), 0);
        lv_obj_set_style_border_color(g.banner, lv_color_hex(0xFF5A5A), 0);
        lv_obj_set_style_bg_opa(g.banner, LV_OPA_90, 0);
        lv_obj_set_style_border_opa(g.banner, LV_OPA_90, 0);
    } else {
        lv_obj_set_style_bg_color(g.banner, lv_color_hex(0x0F2A16), 0);
        lv_obj_set_style_border_color(g.banner, lv_color_hex(0x4BE37A), 0);
        lv_obj_set_style_bg_opa(g.banner, LV_OPA_70, 0);
        lv_obj_set_style_border_opa(g.banner, LV_OPA_70, 0);
    }
}

// ----------------------------------
// Optional callbacks
// ----------------------------------
extern "C" void ui_set_hose_toggle_callback(ui_hose_toggle_cb_t cb)
{
    s_hose_toggle_cb = cb;
}

extern "C" void ui_set_hose_setpoint_callback(ui_hose_setpoint_cb_t cb)
{
    s_hose_setpoint_cb = cb;
}

// ----------------------------------
// Gauge cards
// ----------------------------------
static void make_hp_card(
    lv_obj_t *parent,
    const char *title,
    lv_obj_t **out_arc,
    lv_obj_t **out_psi,
    lv_obj_t **out_temp,
    lv_obj_t **out_lowtemp)
{
    lv_obj_t *card = card_begin(parent);

    // Fixed (non-flex) layout inside the card.
    // Requirement: gauge centered; reading prominent and centered; unit directly under;
    // temps bottom-center.
    lv_obj_set_layout(card, LV_LAYOUT_NONE);

    lv_obj_t *t = lv_label_create(card);
    lv_label_set_text(t, title);
    style_title(t);
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *arc = lv_arc_create(card);
    noscroll(arc);
    // Sized to look good in your 800x1280 portrait cards.
    lv_obj_set_size(arc, 250, 250);
    lv_obj_align(arc, LV_ALIGN_TOP_MID, 0, 52);

    lv_arc_set_range(arc, 0, 1600);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_arc_set_value(arc, 0);
    arc_make_visual_only(arc);

    lv_obj_t *psi = lv_label_create(card);
    lv_label_set_text(psi, "0\nPSI");
    style_value_big(psi);
    lv_obj_set_style_text_line_space(psi, 4, 0);
    lv_obj_align(psi, LV_ALIGN_TOP_MID, 0, 140);

    lv_obj_t *temp = lv_label_create(card);
    lv_label_set_text(temp, "0.0 \xC2\xB0""F");
    style_value_med(temp);
    lv_obj_align(temp, LV_ALIGN_BOTTOM_MID, 0, -30);

    lv_obj_t *lowt = lv_label_create(card);
    lv_label_set_text(lowt, "Low: 0.0 \xC2\xB0""F");
    lv_obj_set_style_text_align(lowt, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(lowt, LV_OPA_80, 0);
    lv_obj_set_width(lowt, lv_pct(100));
    lv_obj_align(lowt, LV_ALIGN_BOTTOM_MID, 0, -6);

    if(out_arc)     *out_arc = arc;
    if(out_psi)     *out_psi = psi;
    if(out_temp)    *out_temp = temp;
    if(out_lowtemp) *out_lowtemp = lowt;
}

static void make_small_gauge_card(
    lv_obj_t *parent,
    const char *title,
    int32_t minv,
    int32_t maxv,
    lv_obj_t **out_arc,
    lv_obj_t **out_psi,
    lv_obj_t **out_temp_optional)
{
    lv_obj_t *card = card_begin(parent);

    // Fixed (non-flex) layout inside the card.
    lv_obj_set_layout(card, LV_LAYOUT_NONE);

    lv_obj_t *t = lv_label_create(card);
    lv_label_set_text(t, title);
    style_title(t);
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *arc = lv_arc_create(card);
    noscroll(arc);
    lv_obj_set_size(arc, 200, 200);
    lv_obj_align(arc, LV_ALIGN_TOP_MID, 0, 48);

    lv_arc_set_range(arc, minv, maxv);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_arc_set_value(arc, minv);
    arc_make_visual_only(arc);

    lv_obj_t *psi = lv_label_create(card);
    lv_label_set_text(psi, "0\nPSI");
    // On small cards we must show PSI *and* temperature without overlap.
    // Use a slightly smaller font than the big cards, but keep it bold/legible.
    style_value_med(psi);
    lv_obj_set_style_text_line_space(psi, 2, 0);
    // Center PSI within the arc (not the card) so it sits correctly on ISO/RESIN LOW, GUN AIR, PRIMARY AIR.
    lv_obj_set_width(psi, 180);
    lv_obj_set_style_text_align(psi, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(psi, arc, LV_ALIGN_CENTER, 0, -12);

    lv_obj_t *temp = nullptr;
    if(out_temp_optional) {
        temp = lv_label_create(card);
        lv_label_set_text(temp, "0.0 \xC2\xB0""F");
        // Temperature is secondary to PSI, but still needs to be easy to read.
        style_value_med(temp);
        lv_obj_set_width(temp, 180);
        lv_obj_set_style_text_align(temp, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align_to(temp, arc, LV_ALIGN_CENTER, 0, 56);
        *out_temp_optional = temp;
    }

    if(out_arc) *out_arc = arc;
    if(out_psi) *out_psi = psi;
}

// ----------------------------------
// Ratio card (centered spine)
// ----------------------------------
static void make_ratio_card(lv_obj_t *parent)
{
    lv_obj_t *card = card_begin(parent);

    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *t = lv_label_create(card);
    lv_label_set_text(t, "RATIO");
    style_title(t);

    // Tall bar in the center
    g.ratio_bar_bg = lv_obj_create(card);
    noscroll(g.ratio_bar_bg);
    lv_obj_set_style_radius(g.ratio_bar_bg, 18, 0);
    lv_obj_set_style_bg_opa(g.ratio_bar_bg, LV_OPA_40, 0);
    lv_obj_set_style_bg_color(g.ratio_bar_bg, lv_color_hex(0x24304A), 0);
    lv_obj_set_style_border_width(g.ratio_bar_bg, 0, 0);
    lv_obj_set_style_pad_all(g.ratio_bar_bg, 0, 0);

    // Wider than last iteration; looks better from distance
    lv_obj_set_size(g.ratio_bar_bg, lv_pct(55), lv_pct(72));
    lv_obj_set_flex_grow(g.ratio_bar_bg, 1);

    g.ratio_bar_fill = lv_obj_create(g.ratio_bar_bg);
    noscroll(g.ratio_bar_fill);
    lv_obj_set_style_radius(g.ratio_bar_fill, 18, 0);
    lv_obj_set_style_bg_opa(g.ratio_bar_fill, LV_OPA_90, 0);
    lv_obj_set_style_bg_color(g.ratio_bar_fill, lv_color_hex(0x2D73FF), 0);
    lv_obj_set_style_border_width(g.ratio_bar_fill, 0, 0);

    // Initialize centered baseline chunk
    lv_obj_set_size(g.ratio_bar_fill, lv_pct(100), lv_pct(18));
    lv_obj_align(g.ratio_bar_fill, LV_ALIGN_CENTER, 0, 0);

    g.lbl_ratio = lv_label_create(card);
    lv_label_set_text(g.lbl_ratio, "1.00");
    style_value_big(g.lbl_ratio);
}

// ----------------------------------
// Status + E-Stop cards (center column)
// ----------------------------------
static void make_status_card(lv_obj_t *parent)
{
    lv_obj_t *card = card_begin(parent);

    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *t = lv_label_create(card);
    lv_label_set_text(t, "SYSTEM STATUS");
    style_title(t);

    g.lbl_sys_status = lv_label_create(card);
    lv_label_set_text(g.lbl_sys_status, "OK");
    style_value_big(g.lbl_sys_status);

    g.lbl_interlock = lv_label_create(card);
    lv_label_set_text(g.lbl_interlock, "No interlocks");
    lv_obj_set_style_text_align(g.lbl_interlock, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(g.lbl_interlock, LV_OPA_80, 0);
    lv_obj_set_width(g.lbl_interlock, lv_pct(100));
}

static void estop_event(lv_event_t *e)
{
    (void)e;
    if(!g.lbl_estop) return;

    const char *cur = lv_label_get_text(g.lbl_estop);
    const bool is_reset = (cur && strstr(cur, "RESET") != nullptr);

    if(is_reset) {
        lv_label_set_text(g.lbl_estop, "E-STOP");
        if(g.lbl_interlock) lv_label_set_text(g.lbl_interlock, "");
        ui_set_banner("OK", false);
    } else {
        lv_label_set_text(g.lbl_estop, "RESET");
        if(g.lbl_interlock) lv_label_set_text(g.lbl_interlock, "E-STOP ACTIVE");
        ui_set_banner("E-STOP", true);
    }
}

static void make_estop_card(lv_obj_t *parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 10, 0);
    lv_obj_set_style_pad_row(parent, 8, 0);

    lv_obj_t *title = lv_label_create(parent);
    lv_label_set_text(title, "E-STOP");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title, LV_PCT(100));

    g.btn_estop = lv_btn_create(parent);
    lv_obj_set_size(g.btn_estop, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(g.btn_estop, 1);

    g.lbl_estop = lv_label_create(g.btn_estop);
    lv_label_set_text(g.lbl_estop, "E-STOP");
    // Use an enabled font from lv_conf.h
    lv_obj_set_style_text_font(g.lbl_estop, &lv_font_montserrat_48, 0);
    lv_obj_center(g.lbl_estop);

    lv_obj_add_event_cb(g.btn_estop, estop_event, LV_EVENT_CLICKED, nullptr);
}

// ----------------------------------
// Buttons (Spray / Drum Air)
// ----------------------------------
static lv_obj_t* make_big_button_card(lv_obj_t *parent, const char *title, const char *btn_txt)
{
    lv_obj_t *card = card_begin(parent);

    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *t = lv_label_create(card);
    lv_label_set_text(t, title);
    style_title(t);

    lv_obj_t *btn = lv_btn_create(card);
    noscroll(btn);
    lv_obj_set_width(btn, lv_pct(100));
    lv_obj_set_flex_grow(btn, 1);

    // This helper is used for multiple cards (SPRAY, DRUM AIR, etc.).
    // Do not bind the button label to the E-STOP global label.
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, btn_txt);
    lv_obj_center(lbl);

    return btn;
}


// ----------------------------------
// Hose heat center card with 2 subcards
// ----------------------------------
static void make_hose_subcard(
    lv_obj_t *parent,
    const char *title,
    lv_obj_t **out_status,
    lv_obj_t **out_temp,
    lv_obj_t **out_set,
    lv_obj_t **out_toggle,
    lv_obj_t **out_up,
    lv_obj_t **out_down)
{
    lv_obj_t *card = card_begin(parent);

    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // Keep enough padding so the +/- controls are never clipped.
    lv_obj_set_style_pad_all(card, 8, 0);
    lv_obj_set_style_pad_row(card, 6, 0);

    lv_obj_t *t = lv_label_create(card);
    lv_label_set_text(t, title);
    style_title(t);

    // NOTE: status line removed; the ON/OFF state is shown on the toggle button.
    lv_obj_t *status = nullptr;

    // Temp bigger
    lv_obj_t *temp = lv_label_create(card);
    lv_label_set_text(temp, "0.0 \xC2\xB0""F");
    style_value_big(temp);

    // Setpoint
    lv_obj_t *setp = lv_label_create(card);
    lv_label_set_text(setp, "Set: 0 \xC2\xB0""F");
    style_value_med(setp);

    // Toggle
    lv_obj_t *btn_toggle = lv_btn_create(card);
    noscroll(btn_toggle);
    lv_obj_set_width(btn_toggle, lv_pct(92));
    lv_obj_set_height(btn_toggle, 42);
    lv_obj_t *lbl_t = lv_label_create(btn_toggle);
    lv_label_set_text(lbl_t, "OFF");
    lv_obj_center(lbl_t);

    // Up/Down row
    // TEMP - / TEMP + buttons (pill with rounded outer corners)
    lv_obj_t *pill = lv_obj_create(card);
    // Let flex layout place it naturally so it is never clipped.
    lv_obj_set_size(pill, lv_pct(92), 42);
    lv_obj_set_style_pad_all(pill, 0, 0);
    lv_obj_set_style_pad_gap(pill, 0, 0);
    lv_obj_set_style_border_width(pill, 0, 0);
    lv_obj_set_style_radius(pill, 14, 0);
    lv_obj_set_style_clip_corner(pill, true, 0);
    lv_obj_set_style_bg_color(pill, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_bg_opa(pill, LV_OPA_100, 0);

    lv_obj_set_layout(pill, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(pill, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pill, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // TEMP - on the LEFT
    lv_obj_t *btn_dn = lv_btn_create(pill);
    lv_obj_set_size(btn_dn, lv_pct(49), lv_pct(100));
    lv_obj_set_style_radius(btn_dn, 0, 0);
    lv_obj_set_style_border_width(btn_dn, 0, 0);
    lv_obj_set_style_shadow_width(btn_dn, 0, 0);
    lv_obj_set_style_bg_opa(btn_dn, LV_OPA_TRANSP, 0);
    lv_obj_t *lbl_dn = lv_label_create(btn_dn);
    lv_label_set_text(lbl_dn, "TEMP -");
    lv_obj_set_style_text_color(lbl_dn, lv_color_white(), 0);
    lv_obj_center(lbl_dn);

    // subtle divider (between - and +)
    lv_obj_t *div = lv_obj_create(pill);
    lv_obj_set_size(div, 2, lv_pct(80));
    lv_obj_set_style_bg_color(div, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(div, LV_OPA_60, 0);
    lv_obj_set_style_border_width(div, 0, 0);
    lv_obj_set_style_radius(div, 0, 0);
    lv_obj_set_style_pad_all(div, 0, 0);
    lv_obj_clear_flag(div, LV_OBJ_FLAG_SCROLLABLE);

    // TEMP + on the RIGHT
    lv_obj_t *btn_up = lv_btn_create(pill);
    lv_obj_set_size(btn_up, lv_pct(49), lv_pct(100));
    lv_obj_set_style_radius(btn_up, 0, 0);
    lv_obj_set_style_border_width(btn_up, 0, 0);
    lv_obj_set_style_shadow_width(btn_up, 0, 0);
    lv_obj_set_style_bg_opa(btn_up, LV_OPA_TRANSP, 0);
    lv_obj_t *lbl_up = lv_label_create(btn_up);
    lv_label_set_text(lbl_up, "TEMP +");
    lv_obj_set_style_text_color(lbl_up, lv_color_white(), 0);
    lv_obj_center(lbl_up);
    if(out_status) *out_status = status;
    if(out_temp)   *out_temp   = temp;
    if(out_set)    *out_set    = setp;
    if(out_toggle) *out_toggle = btn_toggle;
    if(out_up)     *out_up     = btn_up;
    if(out_down)   *out_down   = btn_dn;
}

static void make_hose_center_card(lv_obj_t *parent)
{
    lv_obj_t *card = card_begin(parent);

    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(card, 10, 0);

    lv_obj_t *t = lv_label_create(card);
    lv_label_set_text(t, "HOSE HEAT");
    style_title(t);

    // Two subcards in a row
    lv_obj_t *row = lv_obj_create(card);
    transparent_container(row);
    lv_obj_set_size(row, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(row, 1);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(row, 10, 0);

    lv_obj_t *left = lv_obj_create(row);
    transparent_container(left);
    lv_obj_set_size(left, lv_pct(50), lv_pct(100));

    lv_obj_t *right = lv_obj_create(row);
    transparent_container(right);
    lv_obj_set_size(right, lv_pct(50), lv_pct(100));

    make_hose_subcard(left, "HOSE 1",
                      &g.lbl_h1_status, &g.lbl_h1_temp, &g.lbl_h1_set,
                      &g.btn_h1_toggle, &g.btn_h1_up, &g.btn_h1_down);

    make_hose_subcard(right, "HOSE 2",
                      &g.lbl_h2_status, &g.lbl_h2_temp, &g.lbl_h2_set,
                      &g.btn_h2_toggle, &g.btn_h2_up, &g.btn_h2_down);
}

// ----------------------------------
// Events
// ----------------------------------
static void hose1_toggle_event(lv_event_t *e)
{
    (void)e;
    g.hose1_on = !g.hose1_on;
    if(g.lbl_h1_status) lv_label_set_text(g.lbl_h1_status, g.hose1_on ? "ON" : "OFF");
    hose_toggle_button_text(g.btn_h1_toggle, g.hose1_on);
    if(s_hose_toggle_cb) s_hose_toggle_cb(1, g.hose1_on);
}

static void hose2_toggle_event(lv_event_t *e)
{
    (void)e;
    g.hose2_on = !g.hose2_on;
    if(g.lbl_h2_status) lv_label_set_text(g.lbl_h2_status, g.hose2_on ? "ON" : "OFF");
    hose_toggle_button_text(g.btn_h2_toggle, g.hose2_on);
    if(s_hose_toggle_cb) s_hose_toggle_cb(2, g.hose2_on);
}

static void hose1_up_event(lv_event_t *e)
{
    (void)e;
    g.hose1_set_f += 1;
    hose_set_label(g.lbl_h1_set, g.hose1_set_f);
    if(s_hose_setpoint_cb) s_hose_setpoint_cb(1, g.hose1_set_f);
}

static void hose1_down_event(lv_event_t *e)
{
    (void)e;
    g.hose1_set_f -= 1;
    hose_set_label(g.lbl_h1_set, g.hose1_set_f);
    if(s_hose_setpoint_cb) s_hose_setpoint_cb(1, g.hose1_set_f);
}

static void hose2_up_event(lv_event_t *e)
{
    (void)e;
    g.hose2_set_f += 1;
    hose_set_label(g.lbl_h2_set, g.hose2_set_f);
    if(s_hose_setpoint_cb) s_hose_setpoint_cb(2, g.hose2_set_f);
}

static void hose2_down_event(lv_event_t *e)
{
    (void)e;
    g.hose2_set_f -= 1;
    hose_set_label(g.lbl_h2_set, g.hose2_set_f);
    if(s_hose_setpoint_cb) s_hose_setpoint_cb(2, g.hose2_set_f);
}

// ----------------------------------
// Public: Build UI
// ----------------------------------
extern "C" void ui_build_live_view(lv_obj_t *root)
{
    // Fixed-resolution layout (portrait): 800 x 1280
    // Avoid LVGL flex/grid auto-layout here; keep deterministic pixel placement.
    noscroll(root);
    lv_obj_set_size(root, 800, 1280);
    lv_obj_set_style_pad_all(root, 0, 0);

    // Defaults
    g.hose1_set_f = 125;
    g.hose2_set_f = 125;
    g.hose1_on = false;
    g.hose2_on = false;

    // Geometry
    const int PAD = 10;
    const int GAP = 10;
    const int W   = 800;
    const int H   = 1280;

    const int banner_h = 60;
    const int banner_x = PAD;
    const int banner_y = PAD;
    const int banner_w = W - (PAD * 2);

    // Y bands (match current working look)
    const int y_top   = banner_y + banner_h + GAP;
    const int h_top   = 420;
    const int y_ctrl  = y_top + h_top + GAP;
    const int h_ctrl  = 240;
    const int y_row3  = y_ctrl + h_ctrl + GAP;
    const int h_row3  = 250;
    const int y_row4  = y_row3 + h_row3 + GAP;
    const int h_row4  = 250;

    // Column widths
    const int avail_w = banner_w;
    const int ratio_w = 80;
    const int big_w   = (avail_w - ratio_w - (GAP * 2)) / 2;   // left/right
    const int big_h   = h_top;

    // Small cards (3 across)
    const int small_w1 = 253;
    const int small_w2 = 253;
    const int small_w3 = avail_w - (small_w1 + small_w2 + (GAP * 2));
    const int small_h  = h_row3;

    // Controls row (4 across)
    const int ctrl_w1 = 187;
    const int ctrl_w2 = 187;
    const int ctrl_w3 = 187;
    const int ctrl_w4 = avail_w - (ctrl_w1 + ctrl_w2 + ctrl_w3 + (GAP * 3));
    const int ctrl_h  = h_ctrl;

    // Banner
    g.banner = lv_obj_create(root);
    noscroll(g.banner);
    lv_obj_set_pos(g.banner, banner_x, banner_y);
    lv_obj_set_size(g.banner, banner_w, banner_h);
    lv_obj_set_style_radius(g.banner, 12, 0);
    lv_obj_set_style_border_width(g.banner, 2, 0);
    lv_obj_set_style_pad_all(g.banner, 10, 0);

    g.banner_lbl = lv_label_create(g.banner);
    lv_obj_set_width(g.banner_lbl, lv_pct(100));
    lv_obj_set_style_text_align(g.banner_lbl, LV_TEXT_ALIGN_CENTER, 0);

    ui_set_banner("OK", false);

    // Main container (absolute positioning)
    lv_obj_t *main = lv_obj_create(root);
    transparent_container(main);
    lv_obj_set_pos(main, banner_x, y_top);
    lv_obj_set_size(main, banner_w, H - y_top - PAD);

    // --- Top row: ISO HP | RATIO | RESIN HP ---
    lv_obj_t *cell_iso_hp = lv_obj_create(main);
    transparent_container(cell_iso_hp);
    lv_obj_set_pos(cell_iso_hp, 0, 0);
    lv_obj_set_size(cell_iso_hp, big_w, big_h);
    make_hp_card(cell_iso_hp, "ISO PRESSURE (HP)",
                 &g.arc_iso_hp, &g.lbl_iso_hp_psi, &g.lbl_iso_hp_temp, &g.lbl_iso_hp_lowtemp);

    lv_obj_t *cell_ratio = lv_obj_create(main);
    transparent_container(cell_ratio);
    lv_obj_set_pos(cell_ratio, big_w + GAP, 0);
    lv_obj_set_size(cell_ratio, ratio_w, big_h);
    make_ratio_card(cell_ratio);

    lv_obj_t *cell_resin_hp = lv_obj_create(main);
    transparent_container(cell_resin_hp);
    lv_obj_set_pos(cell_resin_hp, big_w + GAP + ratio_w + GAP, 0);
    lv_obj_set_size(cell_resin_hp, big_w, big_h);
    make_hp_card(cell_resin_hp, "RESIN PRESSURE (HP)",
                 &g.arc_resin_hp, &g.lbl_resin_hp_psi, &g.lbl_resin_hp_temp, &g.lbl_resin_hp_lowtemp);

    // --- Controls row: SPRAY | HOSE 1 | HOSE 2 | DRUM AIR ---
    const int ctrl_y_rel = y_ctrl - y_top;

    lv_obj_t *cell_spray = lv_obj_create(main);
    transparent_container(cell_spray);
    lv_obj_set_pos(cell_spray, 0, ctrl_y_rel);
    lv_obj_set_size(cell_spray, ctrl_w1, ctrl_h);
    g.btn_spray = make_big_button_card(cell_spray, "SPRAY", "TOGGLE");

    lv_obj_t *cell_h1 = lv_obj_create(main);
    transparent_container(cell_h1);
    lv_obj_set_pos(cell_h1, ctrl_w1 + GAP, ctrl_y_rel);
    lv_obj_set_size(cell_h1, ctrl_w2, ctrl_h);
    make_hose_subcard(cell_h1, "HOSE 1 HEAT",
                      &g.lbl_h1_status, &g.lbl_h1_temp, &g.lbl_h1_set,
                      &g.btn_h1_toggle, &g.btn_h1_up, &g.btn_h1_down);

    lv_obj_t *cell_h2 = lv_obj_create(main);
    transparent_container(cell_h2);
    lv_obj_set_pos(cell_h2, ctrl_w1 + GAP + ctrl_w2 + GAP, ctrl_y_rel);
    lv_obj_set_size(cell_h2, ctrl_w3, ctrl_h);
    make_hose_subcard(cell_h2, "HOSE 2 HEAT",
                      &g.lbl_h2_status, &g.lbl_h2_temp, &g.lbl_h2_set,
                      &g.btn_h2_toggle, &g.btn_h2_up, &g.btn_h2_down);

    lv_obj_t *cell_drum = lv_obj_create(main);
    transparent_container(cell_drum);
    lv_obj_set_pos(cell_drum, ctrl_w1 + GAP + ctrl_w2 + GAP + ctrl_w3 + GAP, ctrl_y_rel);
    lv_obj_set_size(cell_drum, ctrl_w4, ctrl_h);
    g.btn_drum_air = make_big_button_card(cell_drum, "DRUM AIR", "ON/OFF");

    // Init hose labels
    hose_set_label(g.lbl_h1_set, g.hose1_set_f);
    hose_set_label(g.lbl_h2_set, g.hose2_set_f);
    hose_toggle_button_text(g.btn_h1_toggle, g.hose1_on);
    hose_toggle_button_text(g.btn_h2_toggle, g.hose2_on);
    if(g.lbl_h1_status) lv_label_set_text(g.lbl_h1_status, g.hose1_on ? "ON" : "OFF");
    if(g.lbl_h2_status) lv_label_set_text(g.lbl_h2_status, g.hose2_on ? "ON" : "OFF");

    // Wire hose events
    lv_obj_add_event_cb(g.btn_h1_toggle, hose1_toggle_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g.btn_h2_toggle, hose2_toggle_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g.btn_h1_up, hose1_up_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g.btn_h1_down, hose1_down_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g.btn_h2_up, hose2_up_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g.btn_h2_down, hose2_down_event, LV_EVENT_CLICKED, nullptr);

    // --- Row 3 (small cards): ISO LOW | SYSTEM STATUS | RESIN LOW ---
    const int row3_y_rel = y_row3 - y_top;

    lv_obj_t *cell_iso_low = lv_obj_create(main);
    transparent_container(cell_iso_low);
    lv_obj_set_pos(cell_iso_low, 0, row3_y_rel);
    lv_obj_set_size(cell_iso_low, small_w1, small_h);
    make_small_gauge_card(cell_iso_low, "ISO LOW", 0, 500,
                          &g.arc_iso_low, &g.lbl_iso_low_psi, &g.lbl_iso_low_temp);

    lv_obj_t *cell_status = lv_obj_create(main);
    transparent_container(cell_status);
    lv_obj_set_pos(cell_status, small_w1 + GAP, row3_y_rel);
    lv_obj_set_size(cell_status, small_w2, small_h);
    make_status_card(cell_status);

    lv_obj_t *cell_resin_low = lv_obj_create(main);
    transparent_container(cell_resin_low);
    lv_obj_set_pos(cell_resin_low, small_w1 + GAP + small_w2 + GAP, row3_y_rel);
    lv_obj_set_size(cell_resin_low, small_w3, small_h);
    make_small_gauge_card(cell_resin_low, "RESIN LOW", 0, 500,
                          &g.arc_resin_low, &g.lbl_resin_low_psi, &g.lbl_resin_low_temp);

    // --- Row 4 (small cards): GUN AIR | E-STOP / RESET | PRIMARY AIR ---
    const int row4_y_rel = y_row4 - y_top;

    lv_obj_t *cell_gun = lv_obj_create(main);
    transparent_container(cell_gun);
    lv_obj_set_pos(cell_gun, 0, row4_y_rel);
    lv_obj_set_size(cell_gun, small_w1, h_row4);
    make_small_gauge_card(cell_gun, "GUN AIR", 0, 300,
                          &g.arc_gun_air, &g.lbl_gun_air_psi, nullptr);

    lv_obj_t *cell_estop = lv_obj_create(main);
    transparent_container(cell_estop);
    lv_obj_set_pos(cell_estop, small_w1 + GAP, row4_y_rel);
    lv_obj_set_size(cell_estop, small_w2, h_row4);
    make_estop_card(cell_estop);

    lv_obj_t *cell_primary = lv_obj_create(main);
    transparent_container(cell_primary);
    lv_obj_set_pos(cell_primary, small_w1 + GAP + small_w2 + GAP, row4_y_rel);
    lv_obj_set_size(cell_primary, small_w3, h_row4);
    make_small_gauge_card(cell_primary, "PRIMARY AIR", 0, 300,
                          &g.arc_primary_air, &g.lbl_primary_air_psi, nullptr);

    // Final safety: main not scrollable
    noscroll(main);
}

// ----------------------------------
// Public: Update values
// ----------------------------------
extern "C" void ui_update_live_values(
    float iso_hp_psi,      float resin_hp_psi,
    float iso_low_psi,     float resin_low_psi,
    float primary_air_psi, float gun_air_psi,
    float iso_hp_temp_f,   float resin_hp_temp_f,
    float iso_low_temp_f,  float resin_low_temp_f,
    float hose1_temp_f,    float hose2_temp_f,
    float ratio)
{
    char b[64];

    // ISO HP
    if(g.arc_iso_hp) lv_arc_set_value(g.arc_iso_hp, (int32_t)clampf(iso_hp_psi, 0, 1600));
    if(g.lbl_iso_hp_psi) {
        snprintf(b, sizeof(b), "%d\nPSI", (int)lroundf(iso_hp_psi));
        lv_label_set_text(g.lbl_iso_hp_psi, b);
    }
    if(g.lbl_iso_hp_temp) {
        snprintf(b, sizeof(b), "%.1f \xC2\xB0""F", iso_hp_temp_f);
        lv_label_set_text(g.lbl_iso_hp_temp, b);
    }
    if(g.lbl_iso_hp_lowtemp) {
        snprintf(b, sizeof(b), "Low: %.1f \xC2\xB0""F", iso_low_temp_f);
        lv_label_set_text(g.lbl_iso_hp_lowtemp, b);
    }

    // RESIN HP
    if(g.arc_resin_hp) lv_arc_set_value(g.arc_resin_hp, (int32_t)clampf(resin_hp_psi, 0, 1600));
    if(g.lbl_resin_hp_psi) {
        snprintf(b, sizeof(b), "%d\nPSI", (int)lroundf(resin_hp_psi));
        lv_label_set_text(g.lbl_resin_hp_psi, b);
    }
    if(g.lbl_resin_hp_temp) {
        snprintf(b, sizeof(b), "%.1f \xC2\xB0""F", resin_hp_temp_f);
        lv_label_set_text(g.lbl_resin_hp_temp, b);
    }
    if(g.lbl_resin_hp_lowtemp) {
        snprintf(b, sizeof(b), "Low: %.1f \xC2\xB0""F", resin_low_temp_f);
        lv_label_set_text(g.lbl_resin_hp_lowtemp, b);
    }

    // ISO LOW
    if(g.arc_iso_low) lv_arc_set_value(g.arc_iso_low, (int32_t)clampf(iso_low_psi, 0, 500));
    if(g.lbl_iso_low_psi) {
        snprintf(b, sizeof(b), "%d\nPSI", (int)lroundf(iso_low_psi));
        lv_label_set_text(g.lbl_iso_low_psi, b);
    }
    if(g.lbl_iso_low_temp) {
        snprintf(b, sizeof(b), "%.1f \xC2\xB0""F", iso_low_temp_f);
        lv_label_set_text(g.lbl_iso_low_temp, b);
    }

    // RESIN LOW
    if(g.arc_resin_low) lv_arc_set_value(g.arc_resin_low, (int32_t)clampf(resin_low_psi, 0, 500));
    if(g.lbl_resin_low_psi) {
        snprintf(b, sizeof(b), "%d\nPSI", (int)lroundf(resin_low_psi));
        lv_label_set_text(g.lbl_resin_low_psi, b);
    }
    if(g.lbl_resin_low_temp) {
        snprintf(b, sizeof(b), "%.1f \xC2\xB0""F", resin_low_temp_f);
        lv_label_set_text(g.lbl_resin_low_temp, b);
    }

    // Air
    if(g.arc_primary_air) lv_arc_set_value(g.arc_primary_air, (int32_t)clampf(primary_air_psi, 0, 300));
    if(g.lbl_primary_air_psi) {
        snprintf(b, sizeof(b), "%d\nPSI", (int)lroundf(primary_air_psi));
        lv_label_set_text(g.lbl_primary_air_psi, b);
    }

    if(g.arc_gun_air) lv_arc_set_value(g.arc_gun_air, (int32_t)clampf(gun_air_psi, 0, 300));
    if(g.lbl_gun_air_psi) {
        snprintf(b, sizeof(b), "%d\nPSI", (int)lroundf(gun_air_psi));
        lv_label_set_text(g.lbl_gun_air_psi, b);
    }

    // Hose temps
    if(g.lbl_h1_temp) {
        snprintf(b, sizeof(b), "%.1f \xC2\xB0""F", hose1_temp_f);
        lv_label_set_text(g.lbl_h1_temp, b);
    }
    if(g.lbl_h2_temp) {
        snprintf(b, sizeof(b), "%.1f \xC2\xB0""F", hose2_temp_f);
        lv_label_set_text(g.lbl_h2_temp, b);
    }

    // Ratio numeric + bar (center baseline, deviates up/down)
    if(g.lbl_ratio) {
        snprintf(b, sizeof(b), "%.2f", ratio);
        lv_label_set_text(g.lbl_ratio, b);
    }

    if(g.ratio_bar_bg && g.ratio_bar_fill) {
        const float dev = clampf(ratio - 1.0f, -0.10f, 0.10f); // +/-10%
        const float mag = fabsf(dev) / 0.10f;

        const int bg_h = lv_obj_get_height(g.ratio_bar_bg);
        const int bg_w = lv_obj_get_width(g.ratio_bar_bg);

        const int min_h = (bg_h * 12) / 100;
        const int max_h = (bg_h * 92) / 100;
        int fill_h = (int)lroundf(min_h + (max_h - min_h) * mag);

        const int center_y = bg_h / 2;
        int y = (dev >= 0) ? (center_y - fill_h) : center_y;

        lv_obj_set_size(g.ratio_bar_fill, bg_w, fill_h);
        lv_obj_set_pos(g.ratio_bar_fill, 0, y);
    }
}
