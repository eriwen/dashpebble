#include "pebble.h"

static Window *window;

static TextLayer *hour_layer;
static TextLayer *minute_layer;
static TextLayer *day_layer;
static TextLayer *date_layer;
static TextLayer *month_layer;
static TextLayer *icon_layer;
static TextLayer *temperature_layer;

static AppSync sync;
static uint8_t sync_buffer[64];

GFont font_date;
GFont font_month;
GFont font_hour;
GFont font_minute;
GFont font_weather_icons;
GFont font_weather_forecast;

enum WeatherKey {
  WEATHER_ICON_KEY = 0x0,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x1   // TUPLE_CSTRING
};

static const char* const WEATHER_ICON_IDS = "IN$60B<!\"#F";

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  static char icon_str[] = "F";
  switch (key) {
    case WEATHER_ICON_KEY:
      snprintf(icon_str, sizeof(icon_str), "%c", WEATHER_ICON_IDS[new_tuple->value->uint8]);
      text_layer_set_text(icon_layer, icon_str);
      break;

    case WEATHER_TEMPERATURE_KEY:
      // App Sync keeps new_tuple in sync_buffer, so we may use it directly
      text_layer_set_text(temperature_layer, new_tuple->value->cstring);
      break;
  }
}

static void send_cmd(void) {
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

void upcase(char *text) {
  for(unsigned short i = 0; i <= strlen(text); i++) {
    if((text[i] > 96) && (text[i] < 123)) {
      text[i] = text[i] - 'a' + 'A';
    }
  }
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  static char hour_text[] = "00";
  static char minute_text[] = ":00";
  static char day_text[] = "MON";
  static char date_text[] = "01";
  static char month_text[] = "JAN";

  char *hour_format;

  if (clock_is_24h_style()) {
    hour_format = "%H";
  } else {
    hour_format = "%l";
  }

  strftime(hour_text, sizeof(hour_text), hour_format, tick_time);
  strftime(minute_text, sizeof(minute_text), ":%M", tick_time);
  strftime(day_text, sizeof(day_text), "%a", tick_time);
  strftime(date_text, sizeof(date_text), "%d", tick_time);
  strftime(month_text, sizeof(month_text), "%b", tick_time);

  upcase(day_text);
  upcase(month_text);

  text_layer_set_text(hour_layer, hour_text);
  text_layer_set_text(minute_layer, minute_text);
  text_layer_set_text(day_layer, day_text);
  text_layer_set_text(date_layer, date_text);
  text_layer_set_text(month_layer, month_text);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_18));
  font_month = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_LIGHT_18));
  font_hour = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49));
  font_minute = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_THIN_SUBSET_49));
  font_weather_icons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CLIMACONS_36));
  font_weather_forecast = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_LIGHT_SUBSET_28));

  // TODO: nested layer and center
  hour_layer = text_layer_create(GRect(0, 6, 65, 168-6));
  text_layer_set_text_color(hour_layer, GColorWhite);
  text_layer_set_background_color(hour_layer, GColorClear);
  text_layer_set_text_alignment(hour_layer, GTextAlignmentRight);
  text_layer_set_font(hour_layer, font_hour);
  layer_add_child(window_layer, text_layer_get_layer(hour_layer));

  minute_layer = text_layer_create(GRect(67, 6, 144-67, 168-6));
  text_layer_set_text_color(minute_layer, GColorWhite);
  text_layer_set_background_color(minute_layer, GColorClear);
  text_layer_set_text_alignment(minute_layer, GTextAlignmentLeft);
  text_layer_set_font(minute_layer, font_minute);
  layer_add_child(window_layer, text_layer_get_layer(minute_layer));

  day_layer = text_layer_create(GRect(0, 62, 60, 168-62));
  text_layer_set_text_color(day_layer, GColorWhite);
  text_layer_set_background_color(day_layer, GColorClear);
  text_layer_set_text_alignment(day_layer, GTextAlignmentRight);
  text_layer_set_font(day_layer, font_month);
  layer_add_child(window_layer, text_layer_get_layer(day_layer));

  date_layer = text_layer_create(GRect(60, 62, 30, 168-62));
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_font(date_layer, font_date);
  layer_add_child(window_layer, text_layer_get_layer(date_layer));

  month_layer = text_layer_create(GRect(90, 62, 144-90, 168-62));
  text_layer_set_text_color(month_layer, GColorWhite);
  text_layer_set_background_color(month_layer, GColorClear);
  text_layer_set_text_alignment(month_layer, GTextAlignmentLeft);
  text_layer_set_font(month_layer, font_month);
  layer_add_child(window_layer, text_layer_get_layer(month_layer));

  icon_layer = text_layer_create(GRect(0, 100, 50, 168-100));
  text_layer_set_text_color(icon_layer, GColorWhite);
  text_layer_set_background_color(icon_layer, GColorClear);
  text_layer_set_text_alignment(icon_layer, GTextAlignmentRight);
  text_layer_set_font(icon_layer, font_weather_icons);
  layer_add_child(window_layer, text_layer_get_layer(icon_layer));

  temperature_layer = text_layer_create(GRect(62, 100, 144-62, 168-100));
  text_layer_set_text_color(temperature_layer, GColorWhite);
  text_layer_set_background_color(temperature_layer, GColorClear);
  text_layer_set_font(temperature_layer, font_weather_forecast);
  text_layer_set_text_alignment(temperature_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(temperature_layer));

  // TODO: Powered by Forecast.io

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  Tuplet initial_values[] = {
    TupletInteger(WEATHER_ICON_KEY, (uint8_t) 0),
    TupletCString(WEATHER_TEMPERATURE_KEY, "---\u00B0F")
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  send_cmd();
}

static void window_unload(Window *window) {
  app_sync_deinit(&sync);

  text_layer_destroy(hour_layer);
  text_layer_destroy(minute_layer);
  text_layer_destroy(day_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(month_layer);
  text_layer_destroy(icon_layer);
  text_layer_destroy(temperature_layer);
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
  fonts_unload_custom_font(font_date);
  fonts_unload_custom_font(font_month);
  fonts_unload_custom_font(font_hour);
  fonts_unload_custom_font(font_minute);
  fonts_unload_custom_font(font_weather_icons);
  fonts_unload_custom_font(font_weather_forecast);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
