#include "weather_layer.h"

void weather_layer_update_proc(WeatherLayer *tl, GContext* ctx) {
  if (tl->background_color != GColorClear) {
    graphics_context_set_fill_color(ctx, tl->background_color);
    graphics_fill_rect(ctx, tl->layer.bounds, 0, GCornerNone);
  }
  graphics_context_set_text_color(ctx, tl->text_color);

  if (tl->icon_text && tl->forecast_text) {
    GRect icon_bounds = tl->layer.bounds;
    GRect forecast_bounds = tl->layer.bounds;

    icon_bounds.size.w = 40;
    forecast_bounds.origin.x = 43;
    forecast_bounds.size.w = 168-43;

    graphics_text_draw(ctx, tl->icon_text, tl->icon_font, icon_bounds,
      tl->overflow_mode, GTextAlignmentRight, tl->layout_cache);
    graphics_text_draw(ctx, tl->forecast_text, tl->forecast_font,
      forecast_bounds, tl->overflow_mode, GTextAlignmentLeft, tl->layout_cache);
  }
}

void weather_layer_set_text(WeatherLayer *tl, char *icon_text, char *forecast_text) {
  tl->icon_text = icon_text;
  tl->forecast_text = forecast_text;

  layer_mark_dirty(&(tl->layer));
}

void weather_layer_set_fonts(WeatherLayer *tl, GFont icon_font, GFont forecast_font) {
  tl->icon_font = icon_font;
  tl->forecast_font = forecast_font;

  if (tl->icon_text && tl->forecast_text) {
    layer_mark_dirty(&(tl->layer));
  }
}

void weather_layer_set_text_color(WeatherLayer *tl, GColor color) {
  tl->text_color = color;

  if (tl->icon_text && tl->forecast_text) {
    layer_mark_dirty(&(tl->layer));
  }
}

void weather_layer_set_background_color(WeatherLayer *tl, GColor color) {
  tl->background_color = color;

  if (tl->icon_text && tl->forecast_text) {
    layer_mark_dirty(&(tl->layer));
  }
}

void weather_layer_init(WeatherLayer *tl, GRect frame) {
  layer_init(&tl->layer, frame);
  tl->layer.update_proc = (LayerUpdateProc)weather_layer_update_proc;
  tl->text_color = GColorWhite;
  tl->background_color = GColorClear;
  tl->overflow_mode = GTextOverflowModeWordWrap;

  tl->icon_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  tl->forecast_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
}
