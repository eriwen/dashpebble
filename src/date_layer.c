#include "date_layer.h"

void date_layer_update_proc(DateLayer *tl, GContext* ctx) {
  if (tl->background_color != GColorClear) {
    graphics_context_set_fill_color(ctx, tl->background_color);
    graphics_fill_rect(ctx, tl->layer.bounds, 0, GCornerNone);
  }
  graphics_context_set_text_color(ctx, tl->text_color);

  if (tl->day_text && tl->date_text && tl->month_text) {
    GSize day_size = graphics_text_layout_get_max_used_size(ctx,
       tl->day_text, tl->day_font, tl->layer.bounds,
       tl->overflow_mode, GTextAlignmentLeft, tl->layout_cache);
    GSize date_size = graphics_text_layout_get_max_used_size(ctx,
       tl->date_text, tl->date_font, tl->layer.bounds,
       tl->overflow_mode, GTextAlignmentLeft, tl->layout_cache);
    GSize month_size = graphics_text_layout_get_max_used_size(ctx,
       tl->month_text, tl->month_font, tl->layer.bounds,
       tl->overflow_mode, GTextAlignmentLeft, tl->layout_cache);

    int width = day_size.w + date_size.w + month_size.w;
    int half = tl->layer.bounds.size.w / 2;
    GRect day_bounds = tl->layer.bounds;
    GRect date_bounds = tl->layer.bounds;
    GRect month_bounds = tl->layer.bounds;

    day_bounds.size.w = half - (width / 2) + day_size.w;
    date_bounds.origin.x = day_bounds.size.w + 1;
    date_bounds.size.w = date_size.w;
    month_bounds.origin.x = day_bounds.size.w + date_bounds.size.w + 5;
    month_bounds.size.w = month_size.w;

    graphics_text_draw(ctx, tl->day_text, tl->day_font, day_bounds,
      tl->overflow_mode, GTextAlignmentRight, tl->layout_cache);
    graphics_text_draw(ctx, tl->date_text, tl->date_font,
      date_bounds, tl->overflow_mode, GTextAlignmentCenter, tl->layout_cache);
    graphics_text_draw(ctx, tl->month_text, tl->month_font,
      month_bounds, tl->overflow_mode, GTextAlignmentLeft, tl->layout_cache);
  }
}

void date_layer_set_text(DateLayer *tl, char *day_text, char *date_text, char *month_text) {
  tl->day_text = day_text;
  tl->date_text = date_text;
  tl->month_text = month_text;

  layer_mark_dirty(&(tl->layer));
}

void date_layer_set_fonts(DateLayer *tl, GFont day_font, GFont date_font, GFont month_font) {
  tl->day_font = day_font;
  tl->date_font = date_font;
  tl->month_font = month_font;

  if (tl->day_text && tl->date_text && tl->month_text) {
    layer_mark_dirty(&(tl->layer));
  }
}

void date_layer_set_text_color(DateLayer *tl, GColor color) {
  tl->text_color = color;

  if (tl->day_text && tl->date_text && tl->month_text) {
    layer_mark_dirty(&(tl->layer));
  }
}

void date_layer_set_background_color(DateLayer *tl, GColor color) {
  tl->background_color = color;

  if (tl->day_text && tl->date_text && tl->month_font) {
    layer_mark_dirty(&(tl->layer));
  }
}

void date_layer_init(DateLayer *tl, GRect frame) {
  layer_init(&tl->layer, frame);
  tl->layer.update_proc = (LayerUpdateProc)date_layer_update_proc;
  tl->text_color = GColorWhite;
  tl->background_color = GColorClear;
  tl->overflow_mode = GTextOverflowModeWordWrap;

  tl->day_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  tl->date_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  tl->month_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
}
