#include <pebble.h>

#define BACKGROUND_COLOR GColorCeleste
#define SELECTOR_COLOR GColorMediumSpringGreen
#define RESISTOR_COLOR GColorLightGray

#define SELECTOR_WIDTH 6 //px
#define NUM_BLOCKS 3  
#define MULTI_CLICK_TIMEOUT 125

static Window *s_window;
static Layer *color_layer;
static TextLayer *value_layer;
const uint8_t resistor_colors[10] = {
  GColorBlackARGB8,            //0: Black
  GColorBulgarianRoseARGB8,    //1: Brown
  GColorRedARGB8,              //2: Red
  GColorOrangeARGB8,           //3: Orange
  GColorYellowARGB8,           //4: Yellow
  GColorIslamicGreenARGB8,     //5: Green
  GColorBlueARGB8,             //6: Blue 
  GColorImperialPurpleARGB8,   //7: Purple
  GColorDarkGrayARGB8,         //8: Gray
  GColorWhiteARGB8             //9: White
};

uint8_t block_values[NUM_BLOCKS] = {0, 0, 0};
uint8_t current_block = 0;
bool editing_color = true;

static void update_value_display(/*char units[]*/){
  static char text_buffer[32],  units[8], dot[2], zero[2];
  
  switch(block_values[2]){
    
    case 0:
      strcpy(units, "\0");
      strcpy(zero, "\0");
      strcpy(dot, "\0");
    break;
    
    case 1:
      strcpy(units, "\0");
      strcpy(zero, "0");
      strcpy(dot, "\0");
    break;
    
    case 2:
      if(block_values[0]){
        strcpy(units, "K");
        strcpy(zero, "\0");
        strcpy(dot, ".");
      }
      else{
        strcpy(units, "\0");
        strcpy(zero, "00");
        strcpy(dot, "\0");
      }
    break;
    
    case 3:
      strcpy(units, "K");
      strcpy(zero, "\0");
      strcpy(dot, "\0");
    break;
    
    case 4:
      strcpy(units, "K");
      strcpy(zero, "0");
      strcpy(dot, "\0");
    break;
    
    case 5:
      if(block_values[0]){
        strcpy(units, "M");
        strcpy(zero, "\0");
        strcpy(dot, ".");
      }
      else{
        strcpy(units, "K");
        strcpy(zero, "00");
        strcpy(dot, "\0");
      }
    break;
    
    case 6:
      strcpy(units, "M");
      strcpy(zero, "\0");
      strcpy(dot, "\0");
    break;
    
    default:
      strcpy(units, "\0");
      strcpy(zero, "\0");
      strcpy(dot, "\0");
    break;
    
  }
  
  if(block_values[0]){
    snprintf(text_buffer, sizeof(text_buffer), "%d%s%d%s%s\nohm", block_values[0], dot, block_values[1], zero, units);
  }
  else{
    snprintf(text_buffer, sizeof(text_buffer), "%d%s%s\nohm", block_values[1], zero, units);
  }
  
  text_layer_set_text(value_layer, text_buffer);
}

static void color_update_proc(Layer *layer, GContext *ctx){
  GRect bounds = layer_get_bounds(layer);
  GRect color_rects[3] = {
    GRect(0, 10, bounds.size.w/3, bounds.size.w/3),
    GRect(bounds.size.w/3, 10, bounds.size.w/3, bounds.size.w/3),
    GRect(2*bounds.size.w/3, 10, bounds.size.w/3, bounds.size.w/3)
  };
  GRect value_rect = GRect(
    10,
    bounds.size.h/2 - 20,
    bounds.size.w - 20,
    bounds.size.h/2 + 15
  );
  
  graphics_context_set_fill_color(ctx, RESISTOR_COLOR);
  graphics_fill_rect(ctx, GRect(0, 10, bounds.size.w, bounds.size.w/3), 0, GCornerNone);
  
  if(editing_color){
    graphics_context_set_fill_color(ctx, SELECTOR_COLOR);
    graphics_fill_rect(ctx, color_rects[current_block], 0, GCornerNone);
  }
  else{
    graphics_context_set_fill_color(ctx, SELECTOR_COLOR);
    graphics_fill_rect(ctx, value_rect, 0, GCornerNone);
    graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
    graphics_fill_rect(ctx, grect_inset(value_rect, GEdgeInsets(SELECTOR_WIDTH)), 0, GCornerNone);
  }
  
  for(int i = 0; i < NUM_BLOCKS; i++){
    graphics_context_set_fill_color(ctx, (GColor)(resistor_colors[block_values[i]]));
    graphics_fill_rect(ctx, grect_inset(color_rects[i], GEdgeInsets(SELECTOR_WIDTH)), 0, GCornerNone);
  }
  
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context){
  if(editing_color){
    int max_val = (current_block == 2) ? 6 : 9;
  
    if (block_values[current_block] == max_val) block_values[current_block] = 0;
    else block_values[current_block]++;
  }
  else{
    if(block_values[1] != 9){
      block_values[1]++;
    }
    else{
      block_values[1] = 0;
      if(block_values[0] != 9){
        block_values[0]++;
      }
      else{
        block_values[0] = 0;
        if(block_values[2] != 6){
          block_values[2]++;
        }
      }
    }
  }
  
  layer_mark_dirty(color_layer);
  update_value_display();
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context){
  if(editing_color){
    if (current_block == 2) current_block = 0;
    else current_block++;
  
    layer_mark_dirty(color_layer);
    update_value_display();
  }
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context){ 
  if(editing_color){
    int max_val = (current_block == 2) ? 6 : 9;
  
    if (block_values[current_block] == 0) block_values[current_block] = max_val;
    else block_values[current_block]--;
  }
  else{
    if(block_values[1] != 0){
      block_values[1]--;
    }
    else{
      block_values[1] = 9;
      if(block_values[0] != 0){
        block_values[0]--;
      }
      else{
        block_values[1] = 9;
        if(block_values[2] != 0){
          block_values[2]--;
        }
      }
    }
  }
  layer_mark_dirty(color_layer);
  update_value_display();
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context){
  editing_color = !editing_color;
  vibes_double_pulse();
  layer_mark_dirty(color_layer);
}

static void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context){}

/*
static void up_multi_click_handler(ClickRecognizerRef recognizer, void *context){
  if(!editing_color){
    if(block_values[0] <= 7) block_values[0] += 2;
    else if(block_values[0] == 8) block_values[0]++;
    layer_mark_dirty(color_layer);
    update_value_display();
  }
}

static void down_multi_click_handler(ClickRecognizerRef recognizer, void *context){
  if(!editing_color){
    if(block_values[0] >= 2) block_values[0] -= 2;
    else if(block_values[0] == 1) block_values[0]--;
    layer_mark_dirty(color_layer);
    update_value_display();
  }
}
*/

static void click_config_provider(Window *window){
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 150, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 150, down_single_click_handler);
  
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, select_long_click_release_handler);
  
  /*
  window_multi_click_subscribe(BUTTON_ID_UP, 2, 0, MULTI_CLICK_TIMEOUT, false, up_multi_click_handler);
  window_multi_click_subscribe(BUTTON_ID_DOWN, 2, 0, MULTI_CLICK_TIMEOUT, false, down_multi_click_handler);
  */
}

static void init(void){
  s_window = window_create();
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);
  
  window_set_click_config_provider(s_window, (ClickConfigProvider) click_config_provider);
  window_stack_push(s_window, true);
  window_set_background_color(s_window, BACKGROUND_COLOR);
  
  color_layer = layer_create(bounds);
  layer_set_update_proc(color_layer, color_update_proc);
  
  value_layer = text_layer_create(GRect(
    0,
    bounds.size.h/2 - 20,
    bounds.size.w,
    bounds.size.h/2
  ));
  
  text_layer_set_font(value_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(value_layer, GTextAlignmentCenter);
  text_layer_set_background_color(value_layer, GColorClear);
  update_value_display();
  
  layer_add_child(window_layer, color_layer);
  layer_add_child(window_layer, text_layer_get_layer(value_layer));
}

static void deinit(void){
  layer_destroy(color_layer);
  text_layer_destroy(value_layer);
  window_destroy(s_window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}
