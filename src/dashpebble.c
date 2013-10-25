#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "http.h"
#include "util.h"
#include "time_layer.h"
#include "date_layer.h"
#include "weather_layer.h"
#include "config.h"

#define MY_UUID {0x91, 0x41, 0xB6, 0x28, 0xBC, 0x89, 0x49, 0x8E, 0xB1, 0x47, 0x0D, 0x60, 0xA6, 0xC5, 0xE9, 0xB8}
PBL_APP_INFO(MY_UUID, "DashPebble", "Eric Wendelin", 0x1, 0x0, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

// POST variables
#define WEATHER_KEY_LATITUDE 1
#define WEATHER_KEY_LONGITUDE 2
#define WEATHER_KEY_UNIT_SYSTEM 3
// Received variables
#define WEATHER_KEY_CURRENT 1
#define WEATHER_KEY_SUMMARY 2

#define WEATHER_HTTP_COOKIE 1949327669

Window window;
TimeLayer time_layer;
DateLayer date_layer;
WeatherLayer weather_layer;

static int our_latitude, our_longitude;
static bool located;
static const char* const WEATHER_ICON_IDS = "IN$60B<!\"#F";

void request_weather();
void handle_timer(AppContextRef app_ctx, AppTimerHandle handle, uint32_t cookie);

void failed(int32_t cookie, int http_status, void* context) {
  if(cookie == 0 || cookie == WEATHER_HTTP_COOKIE) {
    weather_layer_set_text(&weather_layer, "", "---º");
  }
}

void success(int32_t cookie, int http_status, DictionaryIterator* received, void* context) {
  if(cookie != WEATHER_HTTP_COOKIE) return;
  Tuple* data_tuple = dict_find(received, WEATHER_KEY_CURRENT);
  if(data_tuple) {
    uint16_t value = data_tuple->value->int16;
    uint8_t icon_id = value >> 11;

    int16_t temp = value & 0x3ff;
    if(value & 0x400) temp = -temp;

    // TODO: variable units
    static char temp_str[] = "---ºF";
    static char icon_str[] = "F";
    snprintf(temp_str, sizeof(temp_str), "%dºF", temp);
    snprintf(icon_str, sizeof(icon_str), "%c", WEATHER_ICON_IDS[icon_id]);
    weather_layer_set_text(&weather_layer, icon_str, temp_str);
  }
}

void reconnect(void* context) {
  request_weather();
}

void set_timer(AppContextRef ctx) {
  // schedule event in 30 minutes
  app_timer_send_event(ctx, 1800000, 1);
}

void location(float latitude, float longitude, float altitude, float accuracy, void* context) {
  // Fix the floats
  our_latitude = latitude * 10000;
  our_longitude = longitude * 10000;
  located = true;
  request_weather();
  set_timer((AppContextRef)context);
}

GFont font_date;
GFont font_month;
GFont font_hour;
GFont font_minute;
GFont font_weather_icons;
GFont font_weather_forecast;

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
  resource_init_current_app(&APP_RESOURCES);
  window_init(&window, "DashPebble");
  window_stack_push(&window, true);
  window_set_background_color(&window, GColorBlack);
  window_set_fullscreen(&window, true);

  font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_18));
  font_month = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_LIGHT_18));
  font_hour = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49));
  font_minute = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_THIN_SUBSET_49));
  font_weather_icons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CLIMACONS_36));
  font_weather_forecast = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_LIGHT_SUBSET_28));

  time_layer_init(&time_layer, window.layer.frame);
  time_layer_set_fonts(&time_layer, font_hour, font_minute);
  layer_set_frame(&time_layer.layer, GRect(0, 6, 144, 168-6));
  layer_add_child(&window.layer, &time_layer.layer);

  date_layer_init(&date_layer, window.layer.frame);
  date_layer_set_fonts(&date_layer, font_month, font_date, font_month);
  layer_set_frame(&date_layer.layer, GRect(0, 62, 144, 168-62));
  layer_add_child(&window.layer, &date_layer.layer);

  weather_layer_init(&weather_layer, window.layer.frame);
  weather_layer_set_fonts(&weather_layer, font_weather_icons, font_weather_forecast);
  weather_layer_set_text(&weather_layer, "F", "---º");
  // TODO: "Powered by forecast.io"
  layer_set_frame(&weather_layer.layer, GRect(0, 100, 144, 168-100));
  layer_add_child(&window.layer, &weather_layer.layer);

  http_set_app_id(rand());
  http_register_callbacks((HTTPCallbacks){
    .failure=failed,
    .success=success,
    .reconnect=reconnect,
    .location=location
  }, (void*)ctx);

  // Request weather
  located = false;
  request_weather();

  handle_minute_tick(ctx, NULL);
}

void handle_deinit(AppContextRef ctx) {
  fonts_unload_custom_font(font_date);
  fonts_unload_custom_font(font_month);
  fonts_unload_custom_font(font_hour);
  fonts_unload_custom_font(font_minute);
  fonts_unload_custom_font(font_weather_icons);
  fonts_unload_custom_font(font_weather_forecast);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    },
    .timer_handler = handle_timer,
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 124,
        .outbound = 256,
      }
    }
  };
  app_event_loop(params, &handlers);
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
  request_weather();
  if(cookie)
    set_timer(ctx);
}

void request_weather() {
  if(!located) {
    http_location_request();
    return;
  }
  // Build the HTTP request
  DictionaryIterator *body;
  HTTPResult result = http_out_get("http://forecast-service.herokuapp.com/index.php", WEATHER_HTTP_COOKIE, &body);
  if(result != HTTP_OK) {
    weather_layer_set_text(&weather_layer, "", "---º");
    return;
  }
  dict_write_int32(body, WEATHER_KEY_LATITUDE, our_latitude);
  dict_write_int32(body, WEATHER_KEY_LONGITUDE, our_longitude);
  dict_write_cstring(body, WEATHER_KEY_UNIT_SYSTEM, UNIT_SYSTEM);
  // Send it.
  if(http_out_send() != HTTP_OK) {
    weather_layer_set_text(&weather_layer, "", "---º");
    return;
  }
}
