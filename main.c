#include <pebble.h>

#define KEY_INVERT 0

#define COLORS       PBL_IF_COLOR_ELSE(true, false)
#define ANTIALIASING true
#define KEY_COLOR 1

#define HAND_MARGIN  10
#define FINAL_RADIUS 56

#define ANIMATION_DURATION 700
#define ANIMATION_DELAY    800

typedef struct {
  int hours;
  int minutes;
} Time;

static Window *s_main_window;
static Layer *s_canvas_layer;

static GPoint s_center;
static Time s_last_time, s_anim_time;
static int s_radius = 0;
int t_radius = 7;
double u_radius = 3.5;
int c_radius = 2;
static bool s_animating = false;
static bool active;
static GColor ring;
static GColor color;
/*************************** AnimationImplementation **************************/

static void animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  s_animating = false;
}



static void animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  animation_set_curve(anim, AnimationCurveEaseInOut);
  animation_set_implementation(anim, implementation);
  if(handlers) {
    animation_set_handlers(anim, (AnimationHandlers) {
      .started = animation_started,
      .stopped = animation_stopped
    }, NULL);
  }
  animation_schedule(anim);
}



/*


static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Color scheme?
  Tuple *color_red_t = dict_find(iter, KEY_COLOR_RED);
  Tuple *color_green_t = dict_find(iter, KEY_COLOR_GREEN);
  Tuple *color_blue_t = dict_find(iter, KEY_COLOR_BLUE);
  if(color_red_t && color_green_t && color_blue_t) {
    // Apply the color if available
#if defined(PBL_BW)
    window_set_background_color(s_main_window, GColorWhite);
    //text_layer_set_text_color(s_text_layer, GColorBlack);
#elif defined(PBL_COLOR)
    int red = color_red_t->value->int32;
    int green = color_green_t->value->int32;
    int blue = color_blue_t->value->int32;

    // Persist values
    persist_write_int(KEY_COLOR_RED, red);
    persist_write_int(KEY_COLOR_GREEN, green);
    persist_write_int(KEY_COLOR_BLUE, blue);

    GColor bg_color = GColorFromRGB(red, green, blue);
    window_set_background_color(s_main_window, bg_color);
   // text_layer_set_text_color(s_text_layer, gcolor_is_dark(bg_color) ? GColorWhite : GColorBlack);
#endif
  }
}


*/


/************************************ UI **************************************/

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;

  layer_mark_dirty(window_get_root_layer(s_main_window));
  
}

static int hours_to_minutes(int hours_out_of_12) {
  return (int)(float)(((float)hours_out_of_12 / 12.0F) * 60.0F);
}

static void update_proc(Layer *layer, GContext *ctx) {
  // set backround to black
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  
  
  graphics_context_set_stroke_color(ctx, color);
  
  //graphics_context_set_stroke_color(ctx, GColorMediumSpringGreen);
  
  graphics_context_set_stroke_width(ctx, 9);

  graphics_context_set_antialiased(ctx, ANTIALIASING);
/*
  // Inside circle color
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, s_center, s_radius);
*/
  // Draw outline of color circle
  graphics_draw_circle(ctx, s_center, s_radius);

  // Don't use current time while animating
  Time mode_time = (s_animating) ? s_anim_time : s_last_time;

  // Adjust for minutes through the hour
  float minute_angle = TRIG_MAX_ANGLE * mode_time.minutes / 60;
  float hour_angle;
  if(s_animating) {
    // Hours out of 60 for smoothness
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 60;
  } else {
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 12;
  }
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);

  // Plot hands
  GPoint minute_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(s_radius - HAND_MARGIN + 24) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(s_radius - HAND_MARGIN+24) / TRIG_MAX_RATIO) + s_center.y,
  };
  GPoint hour_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(hour_angle)  * (int32_t)(s_radius - (2 * HAND_MARGIN)+6) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(s_radius - (2 * HAND_MARGIN)+6) / TRIG_MAX_RATIO) + s_center.y,
  };

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 8);
  
  
  // Draw hands with positive length only
  if(s_radius > 2 * HAND_MARGIN) {
    graphics_draw_line(ctx, s_center, hour_hand);
  }
  
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 4);
  
  
  if(s_radius > HAND_MARGIN) {
    graphics_draw_line(ctx, s_center, minute_hand);
  }
  //draw white circle
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, s_center, t_radius);
  //draw black circle
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, s_center, u_radius);
  
  
  // Plot hand circle
  GPoint hour_hand_circle = (GPoint) {
    .x = (int16_t)(sin_lookup(hour_angle)  * (int32_t)(s_radius - (2 * HAND_MARGIN)+4) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(s_radius - (2 * HAND_MARGIN)+4) / TRIG_MAX_RATIO) + s_center.y,
  };
  
  graphics_context_set_fill_color(ctx, GColorBlack);
  
if(s_radius > 2 * HAND_MARGIN) {
    graphics_fill_circle(ctx, hour_hand_circle, c_radius);
  }

}



static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  s_center = grect_center_point(&window_bounds);

  s_canvas_layer = layer_create(window_bounds);
  layer_set_update_proc(s_canvas_layer, update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}




static void in_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *tuple = NULL;

  tuple = dict_find(iter, KEY_COLOR);
  color = GColorFromHEX(tuple->value->int32);
/*
  tuple = dict_find(iter, KEY_COLOR);
  if(tuple) {
    APP_LOG(0, "color %ld", tuple->value->int32);
    color = GColorFromHEX(tuple->value->int32);
    }
    */
    
  layer_mark_dirty(s_canvas_layer);
  //layer_mark_dirty(window_get_root_layer(s_main_window));
}

/*********************************** App **************************************/

static int anim_percentage(AnimationProgress dist_normalized, int max) {
  return (int)(float)(((float)dist_normalized / (float)ANIMATION_NORMALIZED_MAX) * (float)max);
}

static void radius_update(Animation *anim, AnimationProgress dist_normalized) {
  s_radius = anim_percentage(dist_normalized, FINAL_RADIUS);

  layer_mark_dirty(window_get_root_layer(s_main_window));
  //layer_mark_dirty(s_canvas_layer);
}

static void hands_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_time.hours = anim_percentage(dist_normalized, hours_to_minutes(s_last_time.hours));
  s_anim_time.minutes = anim_percentage(dist_normalized, s_last_time.minutes);
layer_mark_dirty(window_get_root_layer(s_main_window));
  //layer_mark_dirty(s_canvas_layer);
}

static void init() {
  srand(time(NULL));
  
  ring = GColorFolly;
  active  = persist_read_bool(KEY_INVERT);
  app_message_open(60, 0);
  app_message_register_inbox_received(in_received_handler);
  
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  tick_handler(time_now, MINUTE_UNIT);


  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Prepare animations
  AnimationImplementation radius_impl = {
    .update = radius_update
  };
  animate(ANIMATION_DURATION, ANIMATION_DELAY, &radius_impl, false);

  AnimationImplementation hands_impl = {
    .update = hands_update
  };
  animate(2 * ANIMATION_DURATION, ANIMATION_DELAY, &hands_impl, true);
}

static void deinit() {
  window_destroy(s_main_window);
  app_message_deregister_callbacks();
  persist_write_bool(KEY_INVERT, active);
}

int main() {
  init();
  app_event_loop();
  deinit();
}

