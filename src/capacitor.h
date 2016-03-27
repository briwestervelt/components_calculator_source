#include <pebble.h>

#if defined(PBL_COLOR)
  #define CAPACITOR_BACKGROUND_COLOR GColorCeleste
  #define BAR_COLOR GColorLightGray
  #define CAPACITOR_SELECTOR_COLOR GColorMediumSpringGreen
#else
  #define CAPACITOR_BACKGROUND_COLOR GColorWhite
  #define BAR_COLOR GColorDarkGray
  #define CAPACITOR_SELECTOR_COLOR GColorBlack
#endif

#define CAPACITOR_SELECTOR_WIDTH 6 //px
#define NUMBER_VERTICAL_OFFSET 11 //px
#define CAPACITOR_BLOCK_TOP 10 //px

#define NUM_CAPACITOR_BLOCKS 3

#define CAPACITOR_REPEATING_TIMEOUT 100

static Window *capacitor_window;
static Layer *capacitor_background_layer;
static TextLayer *capacitor_value_layer, *value_text_layer_0, *value_text_layer_1, *value_text_layer_2;

const uint8_t number_values_min[NUM_CAPACITOR_BLOCKS] = {1, 0, 0};

uint8_t number_values[NUM_CAPACITOR_BLOCKS] = {1, 0, 0};
bool editing_value = false;
uint8_t current_value = 0;

static void update_block_display(void){
  static char block_buffer_0[2], block_buffer_1[2], block_buffer_2[2];
  
  snprintf(block_buffer_0, sizeof(block_buffer_0), "%d", number_values[0]);
  snprintf(block_buffer_1, sizeof(block_buffer_1), "%d", number_values[1]);
  snprintf(block_buffer_2, sizeof(block_buffer_2), "%d", number_values[2]);  
  
  text_layer_set_text(value_text_layer_0, block_buffer_0);
  text_layer_set_text(value_text_layer_1, block_buffer_1);
  text_layer_set_text(value_text_layer_2, block_buffer_2);
}

static void update_capacitor_value_display(void){
  static char text_buffer[32], units[2], dot[2], zero[2];
  
  strcpy(zero, (number_values[2] % 3 == 1) ? "0" : "\0");
  strcpy(dot, (number_values[2] % 3 == 2) ? "." : "\0");
  
  if      (number_values[2] < 2)   strcpy(units, "p");
  else if (number_values[2] < 5)   strcpy(units, "n");
  else if (number_values[2] < 8)   strcpy(units, "u");
  else                             strcpy(units, "m");
  
  snprintf(text_buffer, sizeof(text_buffer), "%d%s%d%s%s\nfarad", number_values[0], dot, number_values[1], zero, units);
  text_layer_set_text(capacitor_value_layer, text_buffer);
}

static void update_capacitor_screen(void){
  update_block_display();
  update_capacitor_value_display();
  layer_mark_dirty(capacitor_background_layer);
}

static void capacitor_background_update_proc(Layer *layer, GContext *ctx){
  GRect bounds = layer_get_bounds(layer);
  GRect value_rect = GRect(
    10,
    bounds.size.h/2 - 20,
    bounds.size.w - 20,
    bounds.size.h/2 + 15
  );
  GRect value_rects[3] = {
    GRect(0, CAPACITOR_BLOCK_TOP, bounds.size.w/3, bounds.size.w/3),
    GRect(bounds.size.w/NUM_CAPACITOR_BLOCKS, CAPACITOR_BLOCK_TOP, bounds.size.w/3, bounds.size.w/3),
    GRect(2*bounds.size.w/NUM_CAPACITOR_BLOCKS, CAPACITOR_BLOCK_TOP, bounds.size.w/3, bounds.size.w/3)
  };
  graphics_context_set_fill_color(ctx, BAR_COLOR);
  graphics_fill_rect(ctx, GRect(0, 10, bounds.size.w, bounds.size.w/3), 0, GCornerNone);
  
  for(int i = 0; i < NUM_CAPACITOR_BLOCKS; i++){
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, grect_inset(value_rects[i], GEdgeInsets(CAPACITOR_SELECTOR_WIDTH)), 0, GCornerNone);
  }
  
  if(editing_value){
    graphics_context_set_fill_color(ctx, CAPACITOR_SELECTOR_COLOR);
    graphics_fill_rect(ctx, value_rect, 0, GCornerNone);
    graphics_context_set_fill_color(ctx, CAPACITOR_BACKGROUND_COLOR);
    graphics_fill_rect(ctx, grect_inset(value_rect, GEdgeInsets(CAPACITOR_SELECTOR_WIDTH)), 0, GCornerNone);
  }
  else{
    graphics_context_set_fill_color(ctx, CAPACITOR_SELECTOR_COLOR);
    graphics_fill_rect(ctx, value_rects[current_value], 0, GCornerNone);
  }
  
  for(int i = 0; i < NUM_CAPACITOR_BLOCKS; i++){
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, grect_inset(value_rects[i], GEdgeInsets(CAPACITOR_SELECTOR_WIDTH)), 0, GCornerNone);
  }
}

static void capacitor_up_single_click_handler(ClickRecognizerRef recognizer, void *context){
  int min_val = (current_value == 0) ? 1 : 0;
  
  if(editing_value){
    if(number_values[1] < 9){
      number_values[1]++;
    }
    else if(number_values[0] < 9){
      number_values[1] = number_values_min[1];
      number_values[0]++;
    }
    else if(number_values[2] < 9){
      number_values[1] = number_values_min[1];
      number_values[0] = number_values_min[0];
      number_values[2]++;
    }
  }
  else if (number_values[current_value] == 9) number_values[current_value] = min_val;
  else number_values[current_value]++;
    
  update_capacitor_screen();
}

static void capacitor_select_single_click_handler(ClickRecognizerRef recognizer, void *context){
  if(!editing_value){
    if(current_value == 2) current_value = 0;
    else current_value++;
  
    layer_mark_dirty(capacitor_background_layer);
  }
}

static void capacitor_down_single_click_handler(ClickRecognizerRef recognizer, void *context){
  int min_val = (current_value == 0) ? 1 : 0;
  
  if(editing_value){
    if(number_values[1] > number_values_min[1]){
      number_values[1]--;
    }
    else if(number_values[0] > number_values_min[0]){
      number_values[1] = 9;
      number_values[0]--;
    }
    else if(number_values[2] > number_values_min[2]){
      number_values[1] = 9;
      number_values[0] = 9;
      number_values[2]--;
    }
  }
  else if (number_values[current_value] == min_val) number_values[current_value] = 9;
  else number_values[current_value]--;
    
  update_capacitor_screen();
}

static void capacitor_back_single_click_handler(ClickRecognizerRef recognizer, void *context){
  window_stack_pop(true);
}

static void capacitor_select_long_click_handler(ClickRecognizerRef recognizer, void *context){
  editing_value = !editing_value;
  vibes_double_pulse();
  layer_mark_dirty(capacitor_background_layer);
}

static void capacitor_select_long_click_release_handler(ClickRecognizerRef recognizer, void *context){}

static void capacitor_click_config_provider(Window *window){
  window_single_repeating_click_subscribe(BUTTON_ID_UP, CAPACITOR_REPEATING_TIMEOUT, capacitor_up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, capacitor_select_single_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, CAPACITOR_REPEATING_TIMEOUT, capacitor_down_single_click_handler);
  
  window_single_click_subscribe(BUTTON_ID_BACK, capacitor_back_single_click_handler);
  
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, capacitor_select_long_click_handler, capacitor_select_long_click_release_handler);
}

static void capacitor_window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  window_set_click_config_provider(capacitor_window, (ClickConfigProvider) capacitor_click_config_provider);
  window_set_background_color(capacitor_window, CAPACITOR_BACKGROUND_COLOR);

  capacitor_background_layer = layer_create(bounds);
  layer_set_update_proc(capacitor_background_layer, capacitor_background_update_proc);
  
  GRect letter_rects[3] = {
    GRect(0, CAPACITOR_BLOCK_TOP + NUMBER_VERTICAL_OFFSET, bounds.size.w/3, bounds.size.w/3),
    GRect(bounds.size.w/3, CAPACITOR_BLOCK_TOP + NUMBER_VERTICAL_OFFSET, bounds.size.w/3, bounds.size.w/3),
    GRect(2*bounds.size.w/3, CAPACITOR_BLOCK_TOP + NUMBER_VERTICAL_OFFSET, bounds.size.w/3, bounds.size.w/3)
  };
  
  value_text_layer_0 = text_layer_create(letter_rects[0]);
  value_text_layer_1 = text_layer_create(letter_rects[1]);
  value_text_layer_2 = text_layer_create(letter_rects[2]);
  
  text_layer_set_background_color(value_text_layer_0, GColorClear);
  text_layer_set_background_color(value_text_layer_1, GColorClear);
  text_layer_set_background_color(value_text_layer_2, GColorClear);
  
  text_layer_set_text_alignment(value_text_layer_0, GTextAlignmentCenter);
  text_layer_set_text_alignment(value_text_layer_1, GTextAlignmentCenter);
  text_layer_set_text_alignment(value_text_layer_2, GTextAlignmentCenter);
  
  text_layer_set_font(value_text_layer_0, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(value_text_layer_1, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(value_text_layer_2, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  
  capacitor_value_layer = text_layer_create(GRect(
    0,
    bounds.size.h/2 - 20,
    bounds.size.w,
    bounds.size.h/2 + 20
  ));
  
  text_layer_set_font(capacitor_value_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(capacitor_value_layer, GTextAlignmentCenter);
  text_layer_set_background_color(capacitor_value_layer, GColorClear);
  
  update_capacitor_screen();
  
  layer_add_child(window_layer, capacitor_background_layer);
  layer_add_child(window_layer, text_layer_get_layer(value_text_layer_0));
  layer_add_child(window_layer, text_layer_get_layer(value_text_layer_1));
  layer_add_child(window_layer, text_layer_get_layer(value_text_layer_2));
  layer_add_child(window_layer, text_layer_get_layer(capacitor_value_layer));
}

static void capacitor_window_unload(Window *window){
  layer_destroy(capacitor_background_layer);
  text_layer_destroy(value_text_layer_0);
  text_layer_destroy(value_text_layer_1);
  text_layer_destroy(value_text_layer_2);
  text_layer_destroy(capacitor_value_layer);
}

static void capacitor_init(void){
  capacitor_window = window_create();
  window_set_window_handlers(capacitor_window, (WindowHandlers){
    .load = capacitor_window_load,
    .unload = capacitor_window_unload
  });
}

static void capacitor_deinit(void){
  window_destroy(capacitor_window);
}