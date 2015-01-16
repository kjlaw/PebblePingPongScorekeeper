#include <pebble.h>
  
static Window *s_main_window;
static Window *s_results_window;
static TextLayer *s_top_score;
static TextLayer *s_bottom_score;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static BitmapLayer *s_results_background_layer;
static GBitmap *s_results_background_bitmap;
static TextLayer *s_top_marker;
static TextLayer *s_bottom_marker;

static TextLayer *s_final_top_score;
static TextLayer *s_final_bottom_score;
static TextLayer *s_top_serve_percentage;
static TextLayer *s_bottom_serve_percentage;

static int top_score = 0;
static int bottom_score = 0;
static int serve_switch_counter = 0;

static int top_serves_won = 0;
static int top_serves_total = 0;
static int bottom_serves_won = 0;
static int bottom_serves_total = 0;

static bool top_serve = true;

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Change marker that shows who is serving
  serve_switch_counter++;
  if (serve_switch_counter % 2 == 0) {
    text_layer_set_background_color(s_top_marker, GColorWhite);
    text_layer_set_background_color(s_bottom_marker, GColorBlack);
    top_serve = true;
  } else {
    text_layer_set_background_color(s_top_marker, GColorBlack);
    text_layer_set_background_color(s_bottom_marker, GColorWhite);
    top_serve = false;
  }
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Show results page
  window_stack_push(s_results_window, true);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Increase top score by 1
  if (top_score < 999) {
    top_score++;
    
    static char str[] = "000";
    snprintf(str, sizeof(str), "%d", top_score);
    
    text_layer_set_text(s_top_score, str);
    
    // Keep track of serves won or lost
    if (top_serve) {
      top_serves_won++;
      top_serves_total++;
    } else {
      bottom_serves_total++;
    }
  }
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Increase bottom score by 1
  if (bottom_score < 999) {
    bottom_score++;
    
    static char str[] = "000";
    snprintf(str, sizeof(str), "%d", bottom_score);
    
    text_layer_set_text(s_bottom_score, str);
    
    // Keep track of serves won or lost
    if (!top_serve) {
      bottom_serves_won++;
      bottom_serves_total++;
    } else {
      top_serves_total++;
    }
  }
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Decrease top score by 1
  if (top_score > 0) {
    top_score--;
    
    static char str[] = "000";
    snprintf(str, sizeof(str), "%d", top_score);
    
    text_layer_set_text(s_top_score, str);
    
    // Keep track of serves won or lost
    if (top_serve) {
      top_serves_won--;
      top_serves_total--;
    } else {
      bottom_serves_total--;
    }
  }
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Decrease bottom score by 1
  if (bottom_score > 0) {
    bottom_score--;
    
    static char str[] = "000";
    snprintf(str, sizeof(str), "%d", bottom_score);
    
    text_layer_set_text(s_bottom_score, str);
    
    // Keep track of serves won or lost
    if (!top_serve) {
      bottom_serves_won--;
      bottom_serves_total--;
    } else {
      top_serves_total--;
    }
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
  
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, 500, up_long_click_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 500, down_long_click_handler, NULL);
}

static void results_down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_pop_all(true);
  window_stack_push(s_main_window, true);
}

static void results_click_config_provider(void *context) {
  // Figure out what to do with buttons on results page
  window_single_click_subscribe(BUTTON_ID_DOWN, results_down_single_click_handler);
}

static void main_window_load(Window *window) {
  top_score = 0;
  bottom_score = 0;
  serve_switch_counter = 0;
  
  top_serves_won = 0;
  top_serves_total = 0;
  bottom_serves_won = 0;
  bottom_serves_total = 0;
  
  top_serve = true;
  
  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create top score TextLayer
  s_top_score = text_layer_create(GRect(0, 27, 144, 50));
  text_layer_set_text(s_top_score, "0");
  text_layer_set_font(s_top_score, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_background_color(s_top_score, GColorClear);
  text_layer_set_text_alignment(s_top_score, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_top_score));
  
  // Create bottom score TextLayer
  s_bottom_score = text_layer_create(GRect(0, 91, 144, 50));
  text_layer_set_text(s_bottom_score, "0");
  text_layer_set_font(s_bottom_score, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_background_color(s_bottom_score, GColorClear);
  text_layer_set_text_alignment(s_bottom_score, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bottom_score));
  
  // Create top server marker
  s_top_marker = text_layer_create(GRect(22, 16, 7, 7));
  text_layer_set_background_color(s_top_marker, GColorWhite);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_top_marker));
  
  // Create bottom server marker
  s_bottom_marker = text_layer_create(GRect(22, 145, 7, 7));
  text_layer_set_background_color(s_bottom_marker, GColorBlack);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bottom_marker));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_top_score);
  text_layer_destroy(s_bottom_score);
  text_layer_destroy(s_top_marker);
  text_layer_destroy(s_bottom_marker);
  
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
}

static void results_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
  s_results_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RESULTS_BACKGROUND);
  s_results_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_results_background_layer, s_results_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_results_background_layer));
  
  // Create final top score TextLayer
  s_final_top_score = text_layer_create(GRect(4, 29, 72, 30));
  text_layer_set_font(s_final_top_score, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_background_color(s_final_top_score, GColorClear);
  text_layer_set_text_alignment(s_final_top_score, GTextAlignmentCenter);
  
  static char str1[] = "000";
  snprintf(str1, sizeof(str1), "%d", top_score);
  text_layer_set_text(s_final_top_score, str1);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_final_top_score));
  
  // Create final bottom score TextLayer
  s_final_bottom_score = text_layer_create(GRect(4, 96, 72, 30));
  text_layer_set_font(s_final_bottom_score, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_background_color(s_final_bottom_score, GColorClear);
  text_layer_set_text_alignment(s_final_bottom_score, GTextAlignmentCenter);
  
  static char str2[] = "000";
  snprintf(str2, sizeof(str2), "%d", bottom_score);
  text_layer_set_text(s_final_bottom_score, str2);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_final_bottom_score));
  
  // Create top serve win percentage TextLayer
  s_top_serve_percentage = text_layer_create(GRect(72, 40, 72, 30));
  text_layer_set_font(s_top_serve_percentage, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_background_color(s_top_serve_percentage, GColorClear);
  text_layer_set_text_alignment(s_top_serve_percentage, GTextAlignmentCenter);
  
  int top_percentage = 100 * top_serves_won / top_serves_total;
  static char str3[] = "0000";
  snprintf(str3, sizeof(str3), "%d%%", top_percentage);
  text_layer_set_text(s_top_serve_percentage, str3);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_top_serve_percentage));
  
  // Create bottom serve win percentage TextLayer
  s_bottom_serve_percentage = text_layer_create(GRect(72, 107, 72, 30));
  text_layer_set_font(s_bottom_serve_percentage, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_background_color(s_bottom_serve_percentage, GColorClear);
  text_layer_set_text_alignment(s_bottom_serve_percentage, GTextAlignmentCenter);
  
  int bottom_percentage = 100 * bottom_serves_won / bottom_serves_total;
  static char str4[] = "0000";
  snprintf(str4, sizeof(str4), "%d%%", bottom_percentage);
  text_layer_set_text(s_bottom_serve_percentage, str4);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bottom_serve_percentage));
}

static void results_window_unload(Window *window) {
  // Destroy GBitmap
  gbitmap_destroy(s_results_background_bitmap);
  
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_results_background_layer);
  
  // Destroy TextLayers
  text_layer_destroy(s_final_top_score);
  text_layer_destroy(s_final_bottom_score);
  text_layer_destroy(s_top_serve_percentage);
  text_layer_destroy(s_bottom_serve_percentage);
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set Window to fullscreen, with enabled=true
  window_set_fullscreen(s_main_window, true);
  
  // Set Window to receive click events
  window_set_click_config_provider(s_main_window, click_config_provider);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  
  // Create results Window element
  s_results_window = window_create();
  window_set_fullscreen(s_results_window, true);
  window_set_click_config_provider(s_results_window, results_click_config_provider);
  
  window_set_window_handlers(s_results_window, (WindowHandlers) {
    .load = results_window_load,
    .unload = results_window_unload
  });
}

static void deinit() {
  // Destroy Windows
  window_destroy(s_main_window);
  window_destroy(s_results_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}