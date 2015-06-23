#include "simple_analog.h"
#include "pebble.h"
/*

Steve's first stab at a watch face
5/21/2015

the minute indicator, it doesn't look quite right!
Use a l'il circle

*/
  
static Window *window;
static Layer   *s_hands_layer, 
                *hour_layer;

static BitmapLayer *s_bitmap_layer;
static GBitmap *hour_bitmap;

static uint8_t current_hour;
static uint8_t current_min;
static uint8_t current_sec;

static GPath *s_minute_diamond, *s_second_hand;

static uint32_t hour_pix[] = {
    RESOURCE_ID_IMAGE_TWELVE,                         
    RESOURCE_ID_IMAGE_ONE,
    RESOURCE_ID_IMAGE_TWO,
    RESOURCE_ID_IMAGE_THREE,
    RESOURCE_ID_IMAGE_FOUR, // gets here then craps out(????)
    RESOURCE_ID_IMAGE_FIVE,
    RESOURCE_ID_IMAGE_SIX,
    RESOURCE_ID_IMAGE_SEVEN,
    RESOURCE_ID_IMAGE_EIGHT,
    RESOURCE_ID_IMAGE_NINE,
    RESOURCE_ID_IMAGE_TEN,
    RESOURCE_ID_IMAGE_ELEVEN,
                             };

static GPath *s_minute_diamond;

static void bitmap_update() {
  if (hour_bitmap) {
    gbitmap_destroy(hour_bitmap);
  }
  hour_bitmap = gbitmap_create_with_resource(hour_pix[current_hour%12]);
  bitmap_layer_set_bitmap(s_bitmap_layer, hour_bitmap);  
}

static void hands_update_proc(Layer *layer, GContext *ctx) {

  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  int16_t second_hand_length = 200; //bounds.size.w / 2;

  int32_t second_angle = TRIG_MAX_ANGLE * current_sec/60; 
   
  GPoint second_hand = {
    .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
  };
  
  // second hand
  graphics_context_set_stroke_color(ctx, GColorBlack);  
  //graphics_draw_line(ctx, second_hand, center);
  graphics_context_set_fill_color(ctx, GColorBlack);
  
  gpath_rotate_to(s_second_hand, second_angle);
  gpath_draw_filled(ctx, s_second_hand);
  gpath_draw_outline(ctx, s_second_hand);
  
  gpath_rotate_to(s_minute_diamond, TRIG_MAX_ANGLE * current_min / 60);
  gpath_draw_filled(ctx, s_minute_diamond);
  gpath_draw_outline(ctx, s_minute_diamond);  //???

  bitmap_update(); 
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));  
  current_hour = tick_time->tm_hour;
  current_min = tick_time->tm_min;
  current_sec = tick_time->tm_sec;
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
 
  s_bitmap_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  hour_bitmap = gbitmap_create_with_resource(hour_pix[0]);
  bitmap_layer_set_bitmap(s_bitmap_layer, 
                          hour_bitmap);

  hour_layer = bitmap_layer_get_layer(s_bitmap_layer);
  layer_add_child(window_layer, hour_layer);
  layer_add_child(window_layer, s_hands_layer); // add hands after or you never see it
  APP_LOG(APP_LOG_LEVEL_INFO, "Main window loaded.");
  
}

static void window_unload(Window *window) {
  if (hour_bitmap) {
    gbitmap_destroy(hour_bitmap);
  }
 // bitmap_layer_destroy(s_bitmap_layer); <- note, that caused shit to crash if I did it.
  layer_destroy(hour_layer);   
  layer_destroy(s_hands_layer);
}

static void get_time() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    current_min = t->tm_min;
    current_hour = t->tm_hour;
    current_sec = t->tm_sec;
}

static void init() {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  s_minute_diamond = gpath_create(&MINUTE_DIAMOND_POINTS); //HOUR_HAND_POINTS);
  s_second_hand = gpath_create(&SECOND_HAND_POINTS);
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);
  gpath_move_to(s_minute_diamond, center);
  gpath_move_to(s_second_hand, center);
  get_time();
  bitmap_update();
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

// gbitmap destroy? other destroy? in window unload bitmap is destroyed.
static void deinit() {
  gpath_destroy(s_minute_diamond);  
  gpath_destroy(s_second_hand);
  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}