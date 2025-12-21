#include "ui_main.h"

#include <math.h>
#include <cstdio>   // snprintf

// --------------------------
// Internal state / handles
// --------------------------
typedef struct {
    // Banner
    lv_obj_t *banner;
    lv_obj_t *banner_lbl;
    bool      banner_is_error;

    // HP
    lv_obj_t *arc_iso_hp;
    lv_obj_t *lbl_iso_hp_psi;
    lv_obj_t *lbl_iso_hp_temp;
    lv_obj_t *lbl_iso_low_temp_in_hp;

    lv_obj_t *arc_resin_hp;
    lv_obj_t *lbl_resin_hp_psi;
    lv_obj_t *lbl_resin_hp_temp;
    lv_obj_t *lbl_resin_low_temp_in_hp;

    // Ratio
    lv_obj_t *ratio_bar_bg;
    lv_obj_t *ratio_bar_fill;
    lv_obj_t *lbl_ratio;

    // Buttons (outer)
    lv_obj_t *btn_spray;
    lv_obj_t *btn_drum_air;

    // Hose 1
    lv_obj_t *lbl_h1_status;
    lv_obj_t *lbl_h1_temp;
    lv_obj_t *lbl_h1_set;
    lv_obj_t *btn_h1_toggle;
    lv_obj_t *btn_h1_up;
    lv_obj_t *btn_h1_down;

    // Hose 2
    lv_obj_t *lbl_h2_status;
    lv_obj_t *lbl_h2_temp;
    lv_obj_t *lbl_h2_set;
    lv_obj_t *btn_h2_toggle;
    lv_obj_t *btn_h2_up;
    lv_obj_t *btn_h2_down;

    // Secondary
    lv_obj_t *arc_iso_low;
    lv_obj_t *lbl_iso_low_psi;
    lv_obj_t *lbl_iso_low_temp;

    lv_obj_t *arc_primary_air;
    lv_obj_t *lbl_primary_air_psi;

    lv_obj_t *arc_gun_air;
    lv_obj_t *lbl_gun_air_psi;

    lv_obj_t *arc_resin_low;
    lv_obj_t *lbl_resin_low_psi;
    lv_obj_t *lbl_resin_low_temp;

    // UI setpoints and enable state
    int  hose1_set_f;
    int  hose2_set_f;
    bool hose1_on;
    bool hose2_on;
} ui_live_t;

static ui_live_t g = {0};

static ui_hose_toggle_cb_t   s_hose_toggle_cb   = nullptr;
static ui_hose_setpoint_cb_t s_hose_setpoint_cb = nullptr;

// --------------------------
// Small helpers
// --------------------------
static inline float clampf(float v, float lo, float hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

static void noscroll(lv_obj_t *o)
{
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(o, LV_SCROLLBAR_MODE_OFF);
}

static void style_card(lv_obj_t *o)
{
    lv_obj_set_style_radius(o, 12, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_30, 0);
    lv_obj_set_style_bg_color(o, lv_color_hex(0x0B1220), 0);
    lv_obj_set_style_border_width(o, 2, 0);
    lv_obj_set_style_border_opa(o, LV_OPA_50, 0);
    lv_obj_set_style_border_color(o, lv_color_hex(0x6B7A99), 0);
    lv_obj_set_style_pad_all(o, 10, 0);
}

static void style_title(lv_obj_t *lbl)
{
    lv_obj_set_style_text_opa(lbl, LV_OPA_90, 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lbl, lv_pct(100));
}

static void style_value_big(lv_obj_t *lbl)
{
    lv_obj_set_style_text_opa(lbl, LV_OPA_100, 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lbl, lv_pct(100));
}

static void style_value_med(lv_obj_t *lbl)
{
    // LV_OPA_95 is not guaranteed; LV_OPA_90 is safe in your build.
    lv_obj_set_style_text_opa(lbl, LV_OPA_90, 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lbl, lv_pct(100));
}

static lv_obj_t* make_wrap(lv_obj_t *parent)
{
    lv_obj_t *w = lv_obj_create(parent);
    noscroll(w);
    lv_obj_set_style_bg_opa(w, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(w, 0, 0);
    lv_obj_set_style_pad_all(w, 0, 0);
    return w;
}

static lv_obj_t* make_title(lv_obj_t *parent, const char *txt)
{
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, txt);
    style_title(lbl);
    return lbl;
}

// --------------------------
// Banner
// --------------------------
static void banner_apply_style(bool error)
{
    if(!g.banner) return;
    g.banner_is_error = error;

    if(error) {
        lv_obj_set_style_bg_color(g.banner, lv_color_hex(0x3A0D0D), 0);
        lv_obj_set_style_border_color(g.banner, lv_color_hex(0xFF5A5A), 0);
        lv_obj_set_style_bg_opa(g.banner, LV_OPA_80, 0);
        lv_obj_set_style_border_opa(g.banner, LV_OPA_90, 0);
    } else {
        lv_obj_set_style_bg_color(g.banner, lv_color_hex(0x0F2A16), 0);
        lv_obj_set_style_border_color(g.banner, lv_color_hex(0x4BE37A), 0);
        lv_obj_set_style_bg_opa(g.banner, LV_OPA_70, 0);
        lv_obj_set_style_border_opa(g.banner, LV_OPA_70, 0);
    }
}

extern "C" void ui_set_banner(const char *msg, bool error)
{
    if(!g.banner_lbl) return;
    lv_label_set_text(g.banner_lbl, (msg && msg[0]) ? msg : "OK");
    banner_apply_style(error);
}

// --------------------------
// Hose callbacks
// --------------------------
extern "C" void ui_set_hose_toggle_callback(ui_hose_toggle_cb_t cb)
{
    s_hose_toggle_cb = cb;
}

extern "C" void ui_set_hose_setpoint_callback(ui_hose_setpoint_cb_t cb)
{
    s_hose_setpoint_cb = cb;
}

static void hose_set_label(lv_obj_t *lbl_set, int set_f)
{
    char b[32];
    snprintf(b, sizeof(b), "Set: %d \xC2\xB0""F", set_f); // UTF-8 degree symbol
    lv_label_set_text(lbl_set, b);
}

static void hose_toggle_button_text(lv_obj_t *btn, bool on)
{
    if(!btn) return;
    lv_obj_t *lbl = lv_obj_get_child(btn, 0);
    if(!lbl) return;
    lv_label_set_text(lbl, on ? "HEAT: ON" : "HEAT: OFF");
}

// --------------------------
// HP gauge card
// --------------------------
static void make_hp_gauge_card(
    lv_obj_t *parent,
    const char *title,
    lv_obj_t **out_arc,
    lv_obj_t **out_psi,
    lv_obj_t **out_temp,
    lv_obj_t **out_lowtemp)
{
    lv_obj_t *card = lv_obj_create(parent);
    noscroll(card);
    style_card(card);

    lv_obj_set_size(card, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    make_title(card, title);

    // Wrapper to reduce dead space
    lv_obj_t *arc_wrap = make_wrap(card);
    lv_obj_set_size(arc_wrap, lv_pct(100), lv_pct(72));
    lv_obj_set_flex_grow(arc_wrap, 1);

    lv_obj_t *arc = lv_arc_create(arc_wrap);
    noscroll(arc);
    lv_obj_set_size(arc, lv_pct(92), lv_pct(92));
    lv_obj_center(arc);

    lv_arc_set_range(arc, 0, 1600);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_arc_set_value(arc, 0);

    // Live gauge: non interactive
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    // PSI label centered inside gauge (required)
    lv_obj_t *psi = lv_label_create(arc_wrap);
    lv_label_set_text(psi, "0 PSI");
    style_value_big(psi);
    lv_obj_center(psi);

    // Temps larger than before (required)
    lv_obj_t *temp = lv_label_create(card);
    lv_label_set_text(temp, "0.0 \xC2\xB0""F");
    style_value_med(temp);

    // Low side temp present on HP card (required)
    lv_obj_t *lowt = lv_label_create(card);
    lv_label_set_text(lowt, "Low: 0.0 \xC2\xB0""F");
    lv_obj_set_style_text_opa(lowt, LV_OPA_80, 0);
    lv_obj_set_style_text_align(lowt, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lowt, lv_pct(100));

    if(out_arc)     *out_arc = arc;
    if(out_psi)     *out_psi = psi;
    if(out_temp)    *out_temp = temp;
    if(out_lowtemp) *out_lowtemp = lowt;
}

// --------------------------
// Small gauge card (low/air)
// --------------------------
static void make_small_gauge_card(
    lv_obj_t *parent,
    const char *title,
    int32_t minv,
    int32_t maxv,
    lv_obj_t **out_arc,
    lv_obj_t **out_psi,
    lv_obj_t **out_temp_optional)
{
    lv_obj_t *card = lv_obj_create(parent);
    noscroll(card);
    style_card(card);

    lv_obj_set_size(card, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    make_title(card, title);

    lv_obj_t *arc_wrap = make_wrap(card);
    lv_obj_set_size(arc_wrap, lv_pct(100), lv_pct(62));
    lv_obj_set_flex_grow(arc_wrap, 1);

    lv_obj_t *arc = lv_arc_create(arc_wrap);
    noscroll(arc);
    lv_obj_set_size(arc, lv_pct(88), lv_pct(88));
    lv_obj_center(arc);

    lv_arc_set_range(arc, minv, maxv);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_arc_set_value(arc, minv);

    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *psi = lv_label_create(arc_wrap);
    lv_label_set_text(psi, "0 PSI");
    style_value_med(psi);
    lv_obj_center(psi);

    if(out_temp_optional) {
        lv_obj_t *temp = lv_label_create(card);
        lv_label_set_text(temp, "0.0 \xC2\xB0""F");
        style_value_med(temp);
        *out_temp_optional = temp;
    }

    if(out_arc) *out_arc = arc;
    if(out_psi) *out_psi = psi;
}

// --------------------------
// Ratio column
// --------------------------
static void make_ratio_column(lv_obj_t *parent)
{
    lv_obj_t *card = lv_obj_create(parent);
    noscroll(card);
    style_card(card);

    lv_obj_set_size(card, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    make_title(card, "RATIO");

    g.ratio_bar_bg = lv_obj_create(card);
    noscroll(g.ratio_bar_bg);
    lv_obj_set_style_radius(g.ratio_bar_bg, 18, 0);
    lv_obj_set_style_bg_opa(g.ratio_bar_bg, LV_OPA_40, 0);
    lv_obj_set_style_bg_color(g.ratio_bar_bg, lv_color_hex(0x24304A), 0);
    lv_obj_set_style_border_width(g.ratio_bar_bg, 0, 0);
    lv_obj_set_style_pad_all(g.ratio_bar_bg, 0, 0);
    lv_obj_set_size(g.ratio_bar_bg, lv_pct(45), lv_pct(70));

    g.ratio_bar_fill = lv_obj_create(g.ratio_bar_bg);
    noscroll(g.ratio_bar_fill);
    lv_obj_set_style_radius(g.ratio_bar_fill, 18, 0);
    lv_obj_set_style_bg_opa(g.ratio_bar_fill, LV_OPA_90, 0);
    lv_obj_set_style_bg_color(g.ratio_bar_fill, lv_color_hex(0x2D73FF), 0);
    lv_obj_set_style_border_width(g.ratio_bar_fill, 0, 0);

    // Centered baseline
    lv_obj_set_size(g.ratio_bar_fill, lv_pct(100), lv_pct(20));
    lv_obj_align(g.ratio_bar_fill, LV_ALIGN_CENTER, 0, 0);

    g.lbl_ratio = lv_label_create(card);
    lv_label_set_text(g.lbl_ratio, "1.00");
    style_value_big(g.lbl_ratio);
}

// --------------------------
// Buttons
// --------------------------
static lv_obj_t* make_big_button_card(lv_obj_t *parent, const char *title, const char *btn_txt)
{
    lv_obj_t *card = lv_obj_create(parent);
    noscroll(card);
    style_card(card);

    lv_obj_set_size(card, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    make_title(card, title);

    lv_obj_t *btn = lv_btn_create(card);
    noscroll(btn);
    lv_obj_set_width(btn, lv_pct(95));
    lv_obj_set_flex_grow(btn, 1);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, btn_txt);
    lv_obj_center(lbl);

    return btn;
}

// --------------------------
// Hose heat card (includes ON/OFF toggle + TEMP +/-)
// --------------------------
static void make_hose_heat_card(
    lv_obj_t *parent,
    const char *title,
    lv_obj_t **out_status,
    lv_obj_t **out_temp,
    lv_obj_t **out_set,
    lv_obj_t **out_toggle,
    lv_obj_t **out_up,
    lv_obj_t **out_down)
{
    lv_obj_t *card = lv_obj_create(parent);
    noscroll(card);
    style_card(card);

    lv_obj_set_size(card, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    make_title(card, title);

    // Status indicator (required)
    lv_obj_t *status = lv_label_create(card);
    lv_label_set_text(status, "OFF");
    style_value_med(status);

    // Current temp prominent
    lv_obj_t *temp = lv_label_create(card);
    lv_label_set_text(temp, "0.0 \xC2\xB0""F");
    style_value_big(temp);

    // Setpoint
    lv_obj_t *setp = lv_label_create(card);
    lv_label_set_text(setp, "Set: 0 \xC2\xB0""F");
    style_value_med(setp);

    // Toggle ON/OFF (required)
    lv_obj_t *btn_toggle = lv_btn_create(card);
    noscroll(btn_toggle);
    lv_obj_set_width(btn_toggle, lv_pct(95));
    lv_obj_t *lbl_t = lv_label_create(btn_toggle);
    lv_label_set_text(lbl_t, "HEAT: OFF");
    lv_obj_center(lbl_t);

    // Up/Down row (required)
    lv_obj_t *row = make_wrap(card);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *btn_up = lv_btn_create(row);
    noscroll(btn_up);
    lv_obj_set_size(btn_up, 100, 54);
    lv_obj_t *lbl_up = lv_label_create(btn_up);
    lv_label_set_text(lbl_up, "TEMP +");
    lv_obj_center(lbl_up);

    lv_obj_t *btn_dn = lv_btn_create(row);
    noscroll(btn_dn);
    lv_obj_set_size(btn_dn, 100, 54);
    lv_obj_t *lbl_dn = lv_label_create(btn_dn);
    lv_label_set_text(lbl_dn, "TEMP -");
    lv_obj_center(lbl_dn);

    if(out_status) *out_status = status;
    if(out_temp)   *out_temp   = temp;
    if(out_set)    *out_set    = setp;
    if(out_toggle) *out_toggle = btn_toggle;
    if(out_up)     *out_up     = btn_up;
    if(out_down)   *out_down   = btn_dn;
}

// --------------------------
// Events: hose controls
// --------------------------
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

// --------------------------
// Public: Build UI
// --------------------------
extern "C" void ui_build_live_view(lv_obj_t *root)
{
    // Root must never scroll
    noscroll(root);

    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(root, 10, 0);
    lv_obj_set_style_pad_row(root, 10, 0);
    lv_obj_set_style_pad_column(root, 10, 0);

    // Defaults
    g.hose1_set_f = 125;
    g.hose2_set_f = 125;
    g.hose1_on = false;
    g.hose2_on = false;

    // --- Banner (fixed height) ---
    g.banner = lv_obj_create(root);
    noscroll(g.banner);
    lv_obj_set_size(g.banner, lv_pct(100), 58);
    lv_obj_set_style_radius(g.banner, 10, 0);
    lv_obj_set_style_border_width(g.banner, 2, 0);
    lv_obj_set_style_pad_all(g.banner, 10, 0);

    g.banner_lbl = lv_label_create(g.banner);
    lv_obj_set_width(g.banner_lbl, lv_pct(100));
    lv_obj_set_style_text_align(g.banner_lbl, LV_TEXT_ALIGN_CENTER, 0);

    ui_set_banner("OK", false);

    // --- Main content container takes the rest ---
    lv_obj_t *main = lv_obj_create(root);
    noscroll(main);
    lv_obj_set_flex_grow(main, 1);
    lv_obj_set_width(main, lv_pct(100));
    lv_obj_set_style_bg_opa(main, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(main, 0, 0);
    lv_obj_set_style_pad_all(main, 0, 0);
    lv_obj_set_flex_flow(main, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(main, 10, 0);
    lv_obj_set_style_pad_column(main, 10, 0);

    // --- Top HP Row (flex grow: 44) ---
    lv_obj_t *row_hp = make_wrap(main);
    lv_obj_set_flex_flow(row_hp, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row_hp, 10, 0);
    lv_obj_set_height(row_hp, 0);
    lv_obj_set_flex_grow(row_hp, 44);
    lv_obj_set_width(row_hp, lv_pct(100));
    lv_obj_set_flex_align(row_hp, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *hp_iso_wrap = make_wrap(row_hp);
    lv_obj_set_size(hp_iso_wrap, lv_pct(44), lv_pct(100));

    lv_obj_t *hp_ratio_wrap = make_wrap(row_hp);
    lv_obj_set_size(hp_ratio_wrap, lv_pct(12), lv_pct(100));

    lv_obj_t *hp_resin_wrap = make_wrap(row_hp);
    lv_obj_set_size(hp_resin_wrap, lv_pct(44), lv_pct(100));

    make_hp_gauge_card(hp_iso_wrap, "ISO PRESSURE (HP)",
                       &g.arc_iso_hp, &g.lbl_iso_hp_psi, &g.lbl_iso_hp_temp, &g.lbl_iso_low_temp_in_hp);

    make_ratio_column(hp_ratio_wrap);

    make_hp_gauge_card(hp_resin_wrap, "RESIN PRESSURE (HP)",
                       &g.arc_resin_hp, &g.lbl_resin_hp_psi, &g.lbl_resin_hp_temp, &g.lbl_resin_low_temp_in_hp);

    // --- Controls Row (flex grow: 22) ---
    // SPRAY (outer left) | HOSE1 | HOSE2 | DRUM AIR (outer right)
    lv_obj_t *row_ctl = make_wrap(main);
    lv_obj_set_flex_flow(row_ctl, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row_ctl, 10, 0);
    lv_obj_set_height(row_ctl, 0);
    lv_obj_set_flex_grow(row_ctl, 22);
    lv_obj_set_width(row_ctl, lv_pct(100));
    lv_obj_set_flex_align(row_ctl, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *ctl_spray_wrap = make_wrap(row_ctl);
    lv_obj_set_size(ctl_spray_wrap, lv_pct(22), lv_pct(100));

    lv_obj_t *ctl_h1_wrap = make_wrap(row_ctl);
    lv_obj_set_size(ctl_h1_wrap, lv_pct(28), lv_pct(100));

    lv_obj_t *ctl_h2_wrap = make_wrap(row_ctl);
    lv_obj_set_size(ctl_h2_wrap, lv_pct(28), lv_pct(100));

    lv_obj_t *ctl_drum_wrap = make_wrap(row_ctl);
    lv_obj_set_size(ctl_drum_wrap, lv_pct(22), lv_pct(100));

    g.btn_spray    = make_big_button_card(ctl_spray_wrap, "SPRAY", "TOGGLE");
    g.btn_drum_air = make_big_button_card(ctl_drum_wrap,  "DRUM AIR", "ON/OFF");

    make_hose_heat_card(ctl_h1_wrap, "HOSE 1 HEAT",
                        &g.lbl_h1_status, &g.lbl_h1_temp, &g.lbl_h1_set,
                        &g.btn_h1_toggle, &g.btn_h1_up, &g.btn_h1_down);

    make_hose_heat_card(ctl_h2_wrap, "HOSE 2 HEAT",
                        &g.lbl_h2_status, &g.lbl_h2_temp, &g.lbl_h2_set,
                        &g.btn_h2_toggle, &g.btn_h2_up, &g.btn_h2_down);

    // Init hose text
    hose_set_label(g.lbl_h1_set, g.hose1_set_f);
    hose_set_label(g.lbl_h2_set, g.hose2_set_f);
    hose_toggle_button_text(g.btn_h1_toggle, g.hose1_on);
    hose_toggle_button_text(g.btn_h2_toggle, g.hose2_on);

    // Wire events
    lv_obj_add_event_cb(g.btn_h1_toggle, hose1_toggle_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g.btn_h2_toggle, hose2_toggle_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g.btn_h1_up, hose1_up_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g.btn_h1_down, hose1_down_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g.btn_h2_up, hose2_up_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(g.btn_h2_down, hose2_down_event, LV_EVENT_CLICKED, nullptr);

    // --- Bottom grid (flex grow: 34) ---
    lv_obj_t *grid = make_wrap(main);
    lv_obj_set_height(grid, 0);
    lv_obj_set_flex_grow(grid, 34);
    lv_obj_set_width(grid, lv_pct(100));

    static int32_t col_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static int32_t row_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
    lv_obj_set_style_pad_row(grid, 10, 0);
    lv_obj_set_style_pad_column(grid, 10, 0);

    // Create 4 functional cards + 2 reserved (you can repurpose later)
    lv_obj_t *c00 = lv_obj_create(grid); noscroll(c00);
    lv_obj_set_style_bg_opa(c00, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(c00, 0, 0);
    lv_obj_set_style_pad_all(c00, 0, 0);
    lv_obj_set_grid_cell(c00, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    make_small_gauge_card(c00, "ISO LOW", 0, 500, &g.arc_iso_low, &g.lbl_iso_low_psi, &g.lbl_iso_low_temp);

    lv_obj_t *c10 = lv_obj_create(grid); noscroll(c10);
    lv_obj_set_style_bg_opa(c10, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(c10, 0, 0);
    lv_obj_set_style_pad_all(c10, 0, 0);
    lv_obj_set_grid_cell(c10, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    make_small_gauge_card(c10, "PRIMARY AIR", 0, 300, &g.arc_primary_air, &g.lbl_primary_air_psi, nullptr);

    lv_obj_t *c20 = lv_obj_create(grid); noscroll(c20);
    lv_obj_set_style_bg_opa(c20, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(c20, 0, 0);
    lv_obj_set_style_pad_all(c20, 0, 0);
    lv_obj_set_grid_cell(c20, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    make_small_gauge_card(c20, "GUN AIR", 0, 300, &g.arc_gun_air, &g.lbl_gun_air_psi, nullptr);

    lv_obj_t *c01 = lv_obj_create(grid); noscroll(c01);
    lv_obj_set_style_bg_opa(c01, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(c01, 0, 0);
    lv_obj_set_style_pad_all(c01, 0, 0);
    lv_obj_set_grid_cell(c01, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    make_small_gauge_card(c01, "RESIN LOW", 0, 500, &g.arc_resin_low, &g.lbl_resin_low_psi, &g.lbl_resin_low_temp);

    lv_obj_t *c11 = lv_obj_create(grid); noscroll(c11);
    style_card(c11);
    lv_obj_set_grid_cell(c11, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    make_title(c11, "RESERVED");

    lv_obj_t *c21 = lv_obj_create(grid); noscroll(c21);
    style_card(c21);
    lv_obj_set_grid_cell(c21, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    make_title(c21, "RESERVED");
}

// --------------------------
// Public: Update values
// --------------------------
extern "C" void ui_update_live_values(
    float iso_hp_psi,      float resin_hp_psi,
    float iso_low_psi,     float resin_low_psi,
    float primary_air_psi, float gun_air_psi,
    float iso_hp_temp_f,   float resin_hp_temp_f,
    float iso_low_temp_f,  float resin_low_temp_f,
    float hose1_temp_f,    float hose2_temp_f,
    float ratio)
{
    char b[48];

    // HP arcs + PSI
    if(g.arc_iso_hp && g.lbl_iso_hp_psi) {
        lv_arc_set_value(g.arc_iso_hp, (int32_t)clampf(iso_hp_psi, 0, 1600));
        snprintf(b, sizeof(b), "%d PSI", (int)lroundf(iso_hp_psi));
        lv_label_set_text(g.lbl_iso_hp_psi, b);
    }
    if(g.arc_resin_hp && g.lbl_resin_hp_psi) {
        lv_arc_set_value(g.arc_resin_hp, (int32_t)clampf(resin_hp_psi, 0, 1600));
        snprintf(b, sizeof(b), "%d PSI", (int)lroundf(resin_hp_psi));
        lv_label_set_text(g.lbl_resin_hp_psi, b);
    }

    // HP temps
    if(g.lbl_iso_hp_temp) {
        snprintf(b, sizeof(b), "%.1f \xC2\xB0""F", iso_hp_temp_f);
        lv_label_set_text(g.lbl_iso_hp_temp, b);
    }
    if(g.lbl_resin_hp_temp) {
        snprintf(b, sizeof(b), "%.1f \xC2\xB0""F", resin_hp_temp_f);
        lv_label_set_text(g.lbl_resin_hp_temp, b);
    }

    // Low-side temps inside HP cards
    if(g.lbl_iso_low_temp_in_hp) {
        snprintf(b, sizeof(b), "Low: %.1f \xC2\xB0""F", iso_low_temp_f);
        lv_label_set_text(g.lbl_iso_low_temp_in_hp, b);
    }
    if(g.lbl_resin_low_temp_in_hp) {
        snprintf(b, sizeof(b), "Low: %.1f \xC2\xB0""F", resin_low_temp_f);
        lv_label_set_text(g.lbl_resin_low_temp_in_hp, b);
    }

    // Secondary gauges
    if(g.arc_iso_low && g.lbl_iso_low_psi) {
        lv_arc_set_value(g.arc_iso_low, (int32_t)clampf(iso_low_psi, 0, 500));
        snprintf(b, sizeof(b), "%d PSI", (int)lroundf(iso_low_psi));
        lv_label_set_text(g.lbl_iso_low_psi, b);
    }
    if(g.lbl_iso_low_temp) {
        snprintf(b, sizeof(b), "%.1f \xC2\xB0""F", iso_low_temp_f);
        lv_label_set_text(g.lbl_iso_low_temp, b);
    }

    if(g.arc_resin_low && g.lbl_resin_low_psi) {
        lv_arc_set_value(g.arc_resin_low, (int32_t)clampf(resin_low_psi, 0, 500));
        snprintf(b, sizeof(b), "%d PSI", (int)lroundf(resin_low_psi));
        lv_label_set_text(g.lbl_resin_low_psi, b);
    }
    if(g.lbl_resin_low_temp) {
        snprintf(b, sizeof(b), "%.1f \xC2\xB0""F", resin_low_temp_f);
        lv_label_set_text(g.lbl_resin_low_temp, b);
    }

    if(g.arc_primary_air && g.lbl_primary_air_psi) {
        lv_arc_set_value(g.arc_primary_air, (int32_t)clampf(primary_air_psi, 0, 300));
        snprintf(b, sizeof(b), "%d PSI", (int)lroundf(primary_air_psi));
        lv_label_set_text(g.lbl_primary_air_psi, b);
    }

    if(g.arc_gun_air && g.lbl_gun_air_psi) {
        lv_arc_set_value(g.arc_gun_air, (int32_t)clampf(gun_air_psi, 0, 300));
        snprintf(b, sizeof(b), "%d PSI", (int)lroundf(gun_air_psi));
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

    // Ratio numeric + bar (centered baseline; up/down indicates deviation)
    if(g.lbl_ratio) {
        snprintf(b, sizeof(b), "%.2f", ratio);
        lv_label_set_text(g.lbl_ratio, b);
    }

    if(g.ratio_bar_bg && g.ratio_bar_fill) {
        const float dev = clampf(ratio - 1.0f, -0.10f, 0.10f); // +/-10%
        const int bg_h = lv_obj_get_height(g.ratio_bar_bg);
        const int bg_w = lv_obj_get_width(g.ratio_bar_bg);

        const int min_h = (bg_h * 12) / 100;
        const int max_h = (bg_h * 90) / 100;

        const float mag = fabsf(dev) / 0.10f;
        int fill_h = (int)lroundf(min_h + (max_h - min_h) * mag);

        const int center_y = bg_h / 2;
        int y = (dev >= 0) ? (center_y - fill_h) : center_y;

        lv_obj_set_size(g.ratio_bar_fill, bg_w, fill_h);
        lv_obj_set_pos(g.ratio_bar_fill, 0, y);
    }
}
