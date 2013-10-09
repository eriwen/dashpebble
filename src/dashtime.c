#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID {0xD9, 0xA3, 0xDF, 0xC4, 0x77, 0x68, 0x47, 0x52, 0xBB, 0xAE, 0x0D, 0x60, 0xA6, 0xC5, 0xE9, 0xB8}
PBL_APP_INFO(MY_UUID, "DashTime", "Eric Wendelin", 0x1, 0x0, DEFAULT_MENU_ICON, APP_INFO_WATCH_FACE);

Window window;
TextLayer hourLayer;
TextLayer minuteLayer;
TextLayer dayLayer;
TextLayer dateLayer;
TextLayer monthLayer;

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  static char hourText[] = "00";
  static char minuteText[] = ":00";
  static char dayText[] = "MON";
  static char dateText[] = "01";
  static char monthText[] = "JAN";

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
  string_format_time(dayText, sizeof(dayText), "%a", &currentTime);
  string_format_time(dateText, sizeof(dateText), "%d", &currentTime);
  string_format_time(monthText, sizeof(monthText), "%b", &currentTime);

  text_layer_set_text(&hourLayer, hourText);
  text_layer_set_text(&minuteLayer, minuteText);
  text_layer_set_text(&dayLayer, dayText);
  text_layer_set_text(&dateLayer, dateText);
  text_layer_set_text(&monthLayer, monthText);
}

void configure_text_layer(TextLayer textLayer, GRect frame, GTextAlignment alignment, GFont font) {
  text_layer_init(&textLayer, frame);
  text_layer_set_text_alignment(&textLayer, alignment);
  text_layer_set_text_color(&textLayer, GColorWhite);
  text_layer_set_background_color(&textLayer, GColorClear);
  text_layer_set_font(&textLayer, font);
}

void handle_init(AppContextRef ctx) {
  window_init(&window, "DashTime");
  window_stack_push(&window, true);
  window_set_background_color(&window, GColorBlack);

  //resource_init_current_app(&APP_RESOURCES);
  //fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49))

  text_layer_init(&hourLayer, GRect(25, 54, 40, 50));
  text_layer_set_text_alignment(&hourLayer, GTextAlignmentRight);
  text_layer_set_text_color(&hourLayer, GColorWhite);
  text_layer_set_background_color(&hourLayer, GColorClear);
  text_layer_set_font(&hourLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));

  text_layer_init(&minuteLayer, GRect(66, 54, 78, 50));
  text_layer_set_text_alignment(&minuteLayer, GTextAlignmentLeft);
  text_layer_set_text_color(&minuteLayer, GColorWhite);
  text_layer_set_background_color(&minuteLayer, GColorClear);
  text_layer_set_font(&minuteLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));

  text_layer_init(&dayLayer, GRect(0, 100, 77, 30));
  text_layer_set_text_alignment(&dayLayer, GTextAlignmentRight);
  text_layer_set_text_color(&dayLayer, GColorWhite);
  text_layer_set_background_color(&dayLayer, GColorClear);
  text_layer_set_font(&dayLayer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));

  text_layer_init(&dateLayer, GRect(80, 100, 25, 30));
  text_layer_set_text_alignment(&dateLayer, GTextAlignmentCenter);
  text_layer_set_text_color(&dateLayer, GColorWhite);
  text_layer_set_background_color(&dateLayer, GColorClear);
  text_layer_set_font(&dateLayer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));

  text_layer_init(&monthLayer, GRect(105, 100, 30, 30));
  text_layer_set_text_alignment(&monthLayer, GTextAlignmentLeft);
  text_layer_set_text_color(&monthLayer, GColorWhite);
  text_layer_set_background_color(&monthLayer, GColorClear);
  text_layer_set_font(&monthLayer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));

  /*configure_text_layer(hourLayer, GRect(25, 54, 40, 50), GTextAlignmentRight, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  configure_text_layer(minuteLayer, GRect(66, 54, 78, 50), GTextAlignmentLeft, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  configure_text_layer(dayLayer, GRect(0, 100, 77, 30), GTextAlignmentRight, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  configure_text_layer(dateLayer, GRect(80, 100, 25, 30), GTextAlignmentCenter, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  configure_text_layer(monthLayer, GRect(105, 100, 30, 30), GTextAlignmentLeft, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));*/

  handle_minute_tick(ctx, NULL);

  layer_add_child(&window.layer, &hourLayer.layer);
  layer_add_child(&window.layer, &minuteLayer.layer);
  layer_add_child(&window.layer, &dayLayer.layer);
  layer_add_child(&window.layer, &dateLayer.layer);
  layer_add_child(&window.layer, &monthLayer.layer);
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
