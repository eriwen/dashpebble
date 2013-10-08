#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID {0xD9, 0xA3, 0xDF, 0xC4, 0x77, 0x68, 0x47, 0x52, 0xBB, 0xAE, 0x0D, 0x60, 0xA6, 0xC5, 0xE9, 0xB8}
PBL_APP_INFO(MY_UUID, "DashTime", "Eric Wendelin", 0x1, 0x0, DEFAULT_MENU_ICON, APP_INFO_WATCH_FACE);

Window window;
TextLayer hourLayer;
TextLayer minuteLayer;

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  static char hourText[] = "00";
  static char minuteText[] = ":00";

  char *hourFormat;

  PblTm currentTime;
  get_time(&currentTime);

  if (clock_is_24h_style()) {
    hourFormat = "%H";
  } else {
    hourFormat = "%l";
  }

  string_format_time(hourText, sizeof(hourText), hourFormat, &currentTime);
  string_format_time(minuteText, sizeof(minuteText), ":%M", &currentTime);

  text_layer_set_text(&hourLayer, hourText);
  text_layer_set_text(&minuteLayer, minuteText);
}

void handle_init(AppContextRef ctx) {
  window_init(&window, "DashTime");
  window_stack_push(&window, true);
  window_set_background_color(&window, GColorBlack);

  text_layer_init(&hourLayer, GRect(25, 54, 72-25 /* width */, 168-54 /* height */));
  text_layer_set_text_alignment(&hourLayer, GTextAlignmentRight);
  text_layer_set_text_color(&hourLayer, GColorWhite);
  text_layer_set_background_color(&hourLayer, GColorClear);
  text_layer_set_font(&hourLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));

  text_layer_init(&minuteLayer, GRect(75, 54, 144-75 /* width */, 168-54 /* height */));
  text_layer_set_text_alignment(&minuteLayer, GTextAlignmentLeft);
  text_layer_set_text_color(&minuteLayer, GColorWhite);
  text_layer_set_background_color(&minuteLayer, GColorClear);
  text_layer_set_font(&minuteLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));

  handle_minute_tick(ctx, NULL);

  layer_add_child(&window.layer, &hourLayer.layer);
  layer_add_child(&window.layer, &minuteLayer.layer);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,

    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
