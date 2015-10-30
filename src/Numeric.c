#include <pebble.h>

#define MARGIN_TOP 25
#define HOUR_HEIGHT 74
#define MINUTE_HEIGHT 40

static Window *s_main_window;
static BitmapLayer *s_hour_layer;
static TextLayer *s_minute_layer;

static const uint32_t hour_bitmap_ids[] = {
  RESOURCE_ID_HOUR_1,
  RESOURCE_ID_HOUR_2,
  RESOURCE_ID_HOUR_3,
  RESOURCE_ID_HOUR_4,
  RESOURCE_ID_HOUR_5,
  RESOURCE_ID_HOUR_6,
  RESOURCE_ID_HOUR_7,
  RESOURCE_ID_HOUR_8,
  RESOURCE_ID_HOUR_9,
  RESOURCE_ID_HOUR_10,
  RESOURCE_ID_HOUR_11,
  RESOURCE_ID_HOUR_12
};

static GBitmap *hour_bitmaps[ARRAY_LENGTH(hour_bitmap_ids)];

static void update_minute() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  static char s_minute_buffer[3];
  snprintf(s_minute_buffer, sizeof(s_minute_buffer), "%d", tick_time -> tm_min);
  
  text_layer_set_text(s_minute_layer, s_minute_buffer);
}

static void update_hour() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  int hour = tick_time -> tm_hour;
  if (hour > 12) hour -= 12;

  bitmap_layer_set_bitmap(s_hour_layer, hour_bitmaps[hour - 1]);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed | MINUTE_UNIT) update_minute();
  if (units_changed | HOUR_UNIT) update_hour();
}

static void setup_text_layer(TextLayer *text_layer) {
  text_layer_set_background_color(text_layer, GColorBlack);
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
}

static void main_window_load(Window *window) {
  const GFont minute_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LIGHT_40));

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_hour_layer = bitmap_layer_create(GRect(0, MARGIN_TOP, bounds.size.w, HOUR_HEIGHT));
  s_minute_layer = text_layer_create(GRect(0, MARGIN_TOP + HOUR_HEIGHT, bounds.size.w, MINUTE_HEIGHT));

  setup_text_layer(s_minute_layer);
  text_layer_set_font(s_minute_layer, minute_font);

  layer_add_child(window_layer, bitmap_layer_get_layer(s_hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_minute_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_minute_layer);
  bitmap_layer_destroy(s_hour_layer);
  for (uint32_t i = 0; i < ARRAY_LENGTH(hour_bitmaps); i++) {
    gbitmap_destroy(hour_bitmaps[i]);
  }
}

static void init() {
  for (uint32_t i = 0; i < ARRAY_LENGTH(hour_bitmaps); i++) {
    hour_bitmaps[i] = gbitmap_create_with_resource(hour_bitmap_ids[i]);
  }

  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  update_hour();
  update_minute();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}