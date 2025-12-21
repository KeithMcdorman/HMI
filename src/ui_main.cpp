#include "ui_main.h"

// ---------- Styles ----------
static lv_style_t st_screen, st_header, st_card, st_title, st_sub, st_btn, st_btn_pressed, st_muted, st_ratio_ring;

static void styles_init()
{
    lv_style_init(&st_screen);
    lv_style_set_bg_color(&st_screen, lv_color_hex(0x050A14));
    lv_style_set_bg_opa(&st_screen, LV_OPA_COVER);
    lv_style_set_pad_all(&st_screen, 10);

    lv_style_init(&st_header);
    lv_style_set_bg_color(&st_header, lv_color_hex(0x0B1426));
    lv_style_set_bg_opa(&st_header, LV_OPA_COVER);
    lv_style_set_radius(&st_header, 14);
    lv_style_set_pad_all(&st_header, 12);

    lv_style_init(&st_card);
    lv_style_set_bg_color(&st_card, lv_color_hex(0x0B1426));
    lv_style_set_bg_opa(&st_card, LV_OPA_COVER);
    lv_style_set_radius(&st_card, 18);
    lv_style_set_border_color(&st_card, lv_color_hex(0x1A2A44));
    lv_style_set_border_width(&st_card, 2);
    lv_style_set_pad_all(&st_card, 12);

    lv_style_init(&st_title);
    lv_style_set_text_color(&st_title, lv_color_hex(0xEAF2FF));
    lv_style_set_text_letter_space(&st_title, 1);

    lv_style_init(&st_sub);
    lv_style_set_text_color(&st_sub, lv_color_hex(0x7C8FB3));

    lv_style_init(&st_muted);
    lv_style_set_text_color(&st_muted, lv_color_hex(0x7C8FB3));

    lv_style_init(&st_btn);
    lv_style_set_bg_color(&st_btn, lv_color_hex(0x16B35A));
    lv_style_set_bg_opa(&st_btn, LV_OPA_COVER);
    lv_style_set_radius(&st_btn, 22);
    lv_style_set_text_color(&st_btn, lv_color_hex(0x06110A));
    lv_style_set_pad_ver(&st_btn, 10);
    lv_style_set_pad_hor(&st_btn, 14);

    lv_style_init(&st_btn_pressed);
    lv_style_set_bg_color(&st_btn_pressed, lv_color_hex(0x0E7A3D));
    lv_style_set_text_color(&st_btn_pressed, lv_color_hex(0xE9FFF2));

    lv_style_init(&st_ratio_ring);
    lv_style_set_bg_color(&st_ratio_ring, lv_color_hex(0x0A101F));
    lv_style_set_bg_opa(&st_ratio_ring, LV_OPA_COVER);
    lv_style_set_radius(&st_ratio_ring, 999);
    lv_style_set_border_width(&st_ratio_ring, 2);
    lv_style_set_border_color(&st_ratio_ring, lv_color_hex(0x263A5C));
}

// ---------- Press feedback ----------
static void btn_press_feedback_cb(lv_event_t * e)
{
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_PRESSED) {
        lv_obj_add_style(obj, &st_btn_pressed, 0);
    } else if(code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_obj_remove_style(obj, &st_btn_pressed, 0);
    }
}

// ---------- UI helpers ----------
static lv_obj_t * card(lv_obj_t * parent, const char * title)
{
    lv_obj_t * c = lv_obj_create(parent);
    lv_obj_add_style(c, &st_card, 0);

    lv_obj_t * t = lv_label_create(c);
    lv_label_set_text(t, title);
    lv_obj_add_style(t, &st_sub, 0);
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 0);

    return c;
}

// Arc “gauge” placeholder
static lv_obj_t * gauge(lv_obj_t * parent, const char * title, const char * main_txt, const char * sub_txt)
{
    lv_obj_t * c = card(parent, title);

    lv_obj_t * arc = lv_arc_create(c);
    lv_obj_set_size(arc, LV_PCT(100), LV_PCT(100));
    lv_obj_align(arc, LV_ALIGN_CENTER, 0, 8);
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_value(arc, 0);
    lv_arc_set_bg_angles(arc, 135, 45);
    lv_arc_set_angles(arc, 135, 135);
    lv_obj_remove_style(arc, nullptr, LV_PART_KNOB);
    lv_obj_set_style_arc_width(arc, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x203655), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x16B35A), LV_PART_INDICATOR);

    lv_obj_t * v = lv_label_create(c);
    lv_label_set_text(v, main_txt);
    lv_obj_add_style(v, &st_title, 0);
    lv_obj_align(v, LV_ALIGN_CENTER, 0, 18);

    lv_obj_t * s = lv_label_create(c);
    lv_label_set_text(s, sub_txt);
    lv_obj_add_style(s, &st_muted, 0);
    lv_obj_align(s, LV_ALIGN_BOTTOM_MID, 0, -6);

    return c;
}

static lv_obj_t * big_btn(lv_obj_t * parent, const char * txt)
{
    lv_obj_t * b = lv_btn_create(parent);
    lv_obj_add_style(b, &st_btn, 0);
    lv_obj_add_event_cb(b, btn_press_feedback_cb, LV_EVENT_ALL, nullptr);

    lv_obj_t * l = lv_label_create(b);
    lv_label_set_text(l, txt);
    lv_obj_center(l);
    return b;
}

static lv_obj_t * tile_btn(lv_obj_t * parent, const char * top, const char * bottom)
{
    lv_obj_t * b = lv_btn_create(parent);
    lv_obj_add_style(b, &st_btn, 0);
    lv_obj_add_event_cb(b, btn_press_feedback_cb, LV_EVENT_ALL, nullptr);

    lv_obj_t * t1 = lv_label_create(b);
    lv_label_set_text(t1, top);
    lv_obj_set_style_text_color(t1, lv_color_hex(0x062014), 0);
    lv_obj_align(t1, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t * t2 = lv_label_create(b);
    lv_label_set_text(t2, bottom);
    lv_obj_set_style_text_color(t2, lv_color_hex(0x062014), 0);
    lv_obj_align(t2, LV_ALIGN_BOTTOM_MID, 0, -10);

    return b;
}

static lv_obj_t * ratio_ring(lv_obj_t * parent)
{
    lv_obj_t * r = lv_obj_create(parent);
    lv_obj_add_style(r, &st_ratio_ring, 0);

    lv_obj_t * t = lv_label_create(r);
    lv_label_set_text(t, "RATIO");
    lv_obj_add_style(t, &st_title, 0);
    lv_obj_center(t);

    return r;
}

static lv_obj_t * hose_heat_card(lv_obj_t * parent, const char * title, const char * temp_txt)
{
    lv_obj_t * c = card(parent, title);

    lv_obj_t * up = lv_btn_create(c);
    lv_obj_add_style(up, &st_card, 0);
    lv_obj_set_style_bg_color(up, lv_color_hex(0x0A101F), 0);
    lv_obj_set_style_radius(up, 12, 0);
    lv_obj_add_event_cb(up, btn_press_feedback_cb, LV_EVENT_ALL, nullptr);
    lv_obj_t * ul = lv_label_create(up);
    lv_label_set_text(ul, LV_SYMBOL_UP);
    lv_obj_add_style(ul, &st_title, 0);
    lv_obj_center(ul);

    lv_obj_t * mid = lv_label_create(c);
    lv_label_set_text(mid, temp_txt);
    lv_obj_add_style(mid, &st_title, 0);

    lv_obj_t * down = lv_btn_create(c);
    lv_obj_add_style(down, &st_card, 0);
    lv_obj_set_style_bg_color(down, lv_color_hex(0x0A101F), 0);
    lv_obj_set_style_radius(down, 12, 0);
    lv_obj_add_event_cb(down, btn_press_feedback_cb, LV_EVENT_ALL, nullptr);
    lv_obj_t * dl = lv_label_create(down);
    lv_label_set_text(dl, LV_SYMBOL_DOWN);
    lv_obj_add_style(dl, &st_title, 0);
    lv_obj_center(dl);

    // Layout inside this card (manual, no scroll)
    lv_obj_set_size(up,   LV_PCT(100), 48);
    lv_obj_align(up, LV_ALIGN_TOP_MID, 0, 26);

    lv_obj_align(mid, LV_ALIGN_CENTER, 0, 10);

    lv_obj_set_size(down, LV_PCT(100), 48);
    lv_obj_align(down, LV_ALIGN_BOTTOM_MID, 0, -8);

    return c;
}

// ---------- Build screen ----------
extern "C" void ui_main_create(void)
{
    styles_init();

    lv_obj_t * scr = lv_obj_create(nullptr);
    lv_obj_add_style(scr, &st_screen, 0);
    lv_scr_load(scr);

    // Fixed layout: header + main row + bottom grid
    // Screen is 800x1280: we will allocate:
    // Header: 110 px
    // Main row: ~620 px
    // Bottom grid: remaining (~520 px)
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // Header
    lv_obj_t * header = lv_obj_create(scr);
    lv_obj_add_style(header, &st_header, 0);
    lv_obj_set_width(header, LV_PCT(100));
    lv_obj_set_height(header, 110);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "SPRAY FOAM PRESSURE MONITOR");
    lv_obj_add_style(title, &st_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 10, 6);

    lv_obj_t * sub = lv_label_create(header);
    lv_label_set_text(sub, "foam.local • High-pressure & Air System");
    lv_obj_add_style(sub, &st_sub, 0);
    lv_obj_align_to(sub, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

    // Live badge (simple)
    lv_obj_t * live = lv_label_create(header);
    lv_label_set_text(live, "● Live");
    lv_obj_set_style_text_color(live, lv_color_hex(0x20D36B), 0);
    lv_obj_align(live, LV_ALIGN_TOP_RIGHT, -150, 12);

    lv_obj_t * settings = lv_btn_create(header);
    lv_obj_add_style(settings, &st_card, 0);
    lv_obj_set_style_radius(settings, 999, 0);
    lv_obj_set_size(settings, 130, 44);
    lv_obj_align(settings, LV_ALIGN_TOP_RIGHT, -10, 6);
    lv_obj_add_event_cb(settings, btn_press_feedback_cb, LV_EVENT_ALL, nullptr);

    lv_obj_t * sl = lv_label_create(settings);
    lv_label_set_text(sl, "Settings");
    lv_obj_set_style_text_color(sl, lv_color_hex(0xEAF2FF), 0);
    lv_obj_center(sl);

    // Main row container (3 columns)
    lv_obj_t * main_row = lv_obj_create(scr);
    lv_obj_set_style_bg_opa(main_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(main_row, 0, 0);
    lv_obj_set_style_pad_all(main_row, 0, 0);
    lv_obj_set_width(main_row, LV_PCT(100));
    lv_obj_set_height(main_row, 620);
    lv_obj_align(main_row, LV_ALIGN_TOP_MID, 0, 120);
    lv_obj_clear_flag(main_row, LV_OBJ_FLAG_SCROLLABLE);

    static int32_t main_cols[] = { 260, LV_GRID_FR(1), 260, LV_GRID_TEMPLATE_LAST };
    static int32_t main_rows[] = { LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    lv_obj_set_layout(main_row, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(main_row, main_cols, main_rows);
    lv_obj_set_style_pad_gap(main_row, 10, 0);

    // ISO gauge (left)
    lv_obj_t * iso = gauge(main_row, "ISO PRESSURE", "0 PSI", "74.8 °F");
    lv_obj_set_grid_cell(iso, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // Resin gauge (right)
    lv_obj_t * resin = gauge(main_row, "RESIN PRESSURE", "0 PSI", "71.8 °F");
    lv_obj_set_grid_cell(resin, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // Control cluster (center)
    lv_obj_t * ctrl = lv_obj_create(main_row);
    lv_obj_add_style(ctrl, &st_card, 0);
    lv_obj_set_grid_cell(ctrl, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_clear_flag(ctrl, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * enabled = lv_label_create(ctrl);
    lv_label_set_text(enabled, "Hose 2 enabled");
    lv_obj_add_style(enabled, &st_muted, 0);
    lv_obj_align(enabled, LV_ALIGN_TOP_MID, 0, 2);

    // Control grid inside center
    lv_obj_t * cgrid = lv_obj_create(ctrl);
    lv_obj_set_style_bg_opa(cgrid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cgrid, 0, 0);
    lv_obj_set_style_pad_all(cgrid, 0, 0);
    lv_obj_set_size(cgrid, LV_PCT(100), LV_PCT(100));
    lv_obj_align(cgrid, LV_ALIGN_CENTER, 0, 10);
    lv_obj_clear_flag(cgrid, LV_OBJ_FLAG_SCROLLABLE);

    static int32_t ccols[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static int32_t crows[] = { 110, 150, 110, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_layout(cgrid, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(cgrid, ccols, crows);
    lv_obj_set_style_pad_gap(cgrid, 10, 0);

    lv_obj_t * spray = big_btn(cgrid, "SPRAY");
    lv_obj_set_grid_cell(spray, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t * drum = big_btn(cgrid, "DRUM AIR");
    lv_obj_set_grid_cell(drum, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t * h1 = tile_btn(cgrid, "HOSE 1\n70.5 °F", "HEATING!");
    lv_obj_set_grid_cell(h1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t * rr = ratio_ring(cgrid);
    lv_obj_set_grid_cell(rr, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_set_size(rr, 130, 130);

    lv_obj_t * h2 = tile_btn(cgrid, "HOSE 2\n70.1 °F", "HEATING!");
    lv_obj_set_grid_cell(h2, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t * h1on = tile_btn(cgrid, "HOSE 1", "ON");
    lv_obj_set_grid_cell(h1on, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);

    lv_obj_t * h2on = tile_btn(cgrid, "HOSE 2", "ON");
    lv_obj_set_grid_cell(h2on, LV_GRID_ALIGN_STRETCH, 1, 2, LV_GRID_ALIGN_STRETCH, 2, 1);

    // Bottom grid: 3 columns x 2 rows (no scroll)
    lv_obj_t * bottom = lv_obj_create(scr);
    lv_obj_set_style_bg_opa(bottom, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(bottom, 0, 0);
    lv_obj_set_style_pad_all(bottom, 0, 0);
    lv_obj_set_width(bottom, LV_PCT(100));
    lv_obj_set_height(bottom, 520);
    lv_obj_align(bottom, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_clear_flag(bottom, LV_OBJ_FLAG_SCROLLABLE);

    static int32_t bcols[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static int32_t brows[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    lv_obj_set_layout(bottom, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(bottom, bcols, brows);
    lv_obj_set_style_pad_gap(bottom, 10, 0);

    // Row 1
    lv_obj_t * iso_low = gauge(bottom, "ISO LOW", "0 PSI", "72.4 °F");
    lv_obj_set_grid_cell(iso_low, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t * prim = gauge(bottom, "PRIMARY AIR", "0 PSI", "");
    lv_obj_set_grid_cell(prim, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t * gun = gauge(bottom, "GUN AP AIR", "0 PSI", "");
    lv_obj_set_grid_cell(gun, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // Row 2
    lv_obj_t * heat1 = hose_heat_card(bottom, "HOSE HEAT 1", "125 °F");
    lv_obj_set_grid_cell(heat1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t * heat2 = hose_heat_card(bottom, "HOSE HEAT 2", "125 °F");
    lv_obj_set_grid_cell(heat2, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t * resin_low = gauge(bottom, "RESIN LOW", "0 PSI", "72.5 °F");
    lv_obj_set_grid_cell(resin_low, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
}
