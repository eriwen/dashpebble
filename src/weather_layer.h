#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

typedef struct _WeatherLayer {
  Layer layer;
  const char *icon_text;
  const char *forecast_text;
  GFont icon_font;
  GFont forecast_font;
  GTextLayoutCacheRef layout_cache;
  GColor text_color: 2;
  GColor background_color: 2;
  GTextOverflowMode overflow_mode: 2;
} WeatherLayer;

void weather_layer_update_proc(WeatherLayer *tl, GContext* ctx);
void weather_layer_set_text(WeatherLayer *tl, char *icon_text, char *forecast_text);
void weather_layer_set_fonts(WeatherLayer *tl, GFont icon_font, GFont forecast_font);
void weather_layer_init(WeatherLayer *tl, GRect frame);
