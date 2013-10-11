#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

typedef struct _DateLayer {
  Layer layer;
  const char *day_text;
  const char *date_text;
  const char *month_text;
  GFont day_font;
  GFont date_font;
  GFont month_font;
  GTextLayoutCacheRef layout_cache;
  GColor text_color: 2;
  GColor background_color: 2;
  GTextOverflowMode overflow_mode: 2;
} DateLayer;

void date_layer_update_proc(DateLayer *tl, GContext* ctx);
void date_layer_set_text(DateLayer *tl, char *day_text, char *date_text, char *month_text);
void date_layer_set_fonts(DateLayer *tl, GFont day_font, GFont date_font, GFont month_font);
void date_layer_set_text_color(DateLayer *tl, GColor color);
void date_layer_set_background_color(DateLayer *tl, GColor color);
void date_layer_init(DateLayer *tl, GRect frame);
