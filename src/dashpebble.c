#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "time_layer.h"
#include "date_layer.h"

#define MY_UUID {0xD9, 0xA3, 0xDF, 0xC4, 0x77, 0x68, 0x47, 0x52, 0xBB, 0xAE, 0x0D, 0x60, 0xA6, 0xC5, 0xE9, 0xB8}
PBL_APP_INFO(MY_UUID, "DashPebble", "Eric Wendelin", 0x1, 0x0, DEFAULT_MENU_ICON, APP_INFO_WATCH_FACE);

Window window;
TimeLayer time_layer;
DateLayer date_layer;

GFont font_date;
GFont font_month;
GFont font_hour;
GFont font_minute;

void upcase(char *text) {
  for(unsigned short i = 0; i <= strlen(text); i++) {
    if((text[i] > 96) && (text[i] < 123)) {
      text[i] = text[i] - 'a' + 'A';
    }
  }
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  static char hour_text[] = "00";
  static char minute_text[] = ":00";
  static char day_text[] = "MON";
  static char date_text[] = "01";
  static char month_text[] = "JAN";

  (void)ctx;

  char *hour_format;

  PblTm current_time;
  get_time(&current_time);

  if (clock_is_24h_style()) {
    hour_format = "%H";
  } else {
    hour_format = "%l";
  }

  string_format_time(hour_text, sizeof(hour_text), hour_format, &current_time);
  string_format_time(minute_text, sizeof(minute_text), ":%M", &current_time);
  string_format_time(day_text, sizeof(day_text), "%a", &current_time);
  string_format_time(date_text, sizeof(date_text), "%d", &current_time);
  string_format_time(month_text, sizeof(month_text), "%b", &current_time);

  upcase(day_text);
  upcase(month_text);

  time_layer_set_text(&time_layer, hour_text, minute_text);
  date_layer_set_text(&date_layer, day_text, date_text, month_text);
}

void handle_init(AppContextRef ctx) {
  window_init(&window, "DashTime");
  window_stack_push(&window, false);
  window_set_background_color(&window, GColorBlack);
  window_set_fullscreen(&window, true);

  resource_init_current_app(&APP_RESOURCES);

  font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_18));
  font_month = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_LIGHT_18));
  font_hour = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49));
  font_minute = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_THIN_SUBSET_49));

  time_layer_init(&time_layer, window.layer.frame);
  time_layer_set_text_color(&time_layer, GColorWhite);
  time_layer_set_background_color(&time_layer, GColorClear);
  time_layer_set_fonts(&time_layer, font_hour, font_minute);
  layer_set_frame(&time_layer.layer, GRect(0, 6, 144, 168-6));
  layer_add_child(&window.layer, &time_layer.layer);

  date_layer_init(&date_layer, window.layer.frame);
  date_layer_set_text_color(&date_layer, GColorWhite);
  date_layer_set_background_color(&date_layer, GColorClear);
  date_layer_set_fonts(&date_layer, font_month, font_date, font_month);
  layer_set_frame(&date_layer.layer, GRect(0, 62, 144, 168-62));
  layer_add_child(&window.layer, &date_layer.layer);

  handle_minute_tick(ctx, NULL);
}

void handle_deinit(AppContextRef ctx) {
    fonts_unload_custom_font(font_date);
    fonts_unload_custom_font(font_month);
    fonts_unload_custom_font(font_hour);
    fonts_unload_custom_font(font_minute);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,

    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
