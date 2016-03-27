#include <pebble.h>

#if defined(PBL_COLOR)
  #define RESISTOR_BACKGROUND_COLOR GColorCeleste
  #define RESISTOR_SELECTOR_COLOR GColorMediumSpringGreen
  #define RESISTOR_COLOR GColorLightGray
#else
  #define RESISTOR_BACKGROUND_COLOR GColorWhite
  #define RESISTOR_SELECTOR_COLOR GColorBlack
  #define RESISTOR_COLOR GColorDarkGray
#endif

#define RESISTOR_SELECTOR_WIDTH 6 //px
#define BLOCK_TOP 10 //px
#define LETTER_VERTICAL_OFFSET 6 //px

#define NUM_RESISTOR_BLOCKS 3   

#define RESISTOR_REPEATING_TIMEOUT 100

static Window *resistor_window;
static Layer *resistor_color_layer;
static TextLayer *resistor_value_layer,
*block_letter_layer_0, *block_letter_layer_1, *block_letter_layer_2;

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

const uint8_t block_value_max[NUM_RESISTOR_BLOCKS] = {9, 9, 6};
const uint8_t block_value_min[NUM_RESISTOR_BLOCKS] = {1, 0, 0};

uint8_t block_values[NUM_RESISTOR_BLOCKS] = {1, 0, 0};
uint8_t current_block = 0;
char letter_0[8], letter_1[8], letter_2[8];
bool editing_color = true;
  
static void update_letters(void){
 
  switch(block_values[0]){
    case 0:
      strcpy(letter_0, "BLK");
      break;
    case 1:
      strcpy(letter_0, "BRN");
      break;
    case 2:
      strcpy(letter_0, "RED");
      break;
    case 3:
      strcpy(letter_0, "ORG");
      break;
    case 4:
      strcpy(letter_0, "YLW");
      break;
    case 5:
      strcpy(letter_0, "GRN");
      break;
    case 6:
      strcpy(letter_0, "BLU");
      break;
    case 7:
      strcpy(letter_0, "PRPL");
      break;
    case 8:
      strcpy(letter_0, "GRY");
      break;
    case 9:
      strcpy(letter_0, "WHT");
      break;
    default:
      strcpy(letter_0, "X");
      break;
  }
  
    switch(block_values[1]){
    case 0:
      strcpy(letter_1, "BLK");
      break;
    case 1:
      strcpy(letter_1, "BRN");
      break;
    case 2:
      strcpy(letter_1, "RED");
      break;
    case 3:
      strcpy(letter_1, "ORG");
      break;
    case 4:
      strcpy(letter_1, "YLW");
      break;
    case 5:
      strcpy(letter_1, "GRN");
      break;
    case 6:
      strcpy(letter_1, "BLU");
      break;
    case 7:
      strcpy(letter_1, "PRPL");
      break;
    case 8:
      strcpy(letter_1, "GRY");
      break;
    case 9:
      strcpy(letter_1, "WHT");
      break;
    default:
      strcpy(letter_1, "X");
      break;
  }
  
    switch(block_values[2]){
    case 0:
      strcpy(letter_2, "BLK");
      break;
    case 1:
      strcpy(letter_2, "BRN");
      break;
    case 2:
      strcpy(letter_2, "RED");
      break;
    case 3:
      strcpy(letter_2, "ORG");
      break;
    case 4:
      strcpy(letter_2, "YLW");
      break;
    case 5:
      strcpy(letter_2, "GRN");
      break;
    case 6:
      strcpy(letter_2, "BLU");
      break;
    default:
      strcpy(letter_2, "X");
      break;
  }

  text_layer_set_text_color(block_letter_layer_0, 
                            PBL_IF_COLOR_ELSE(gcolor_legible_over((GColor)(resistor_colors[block_values[0]])), GColorBlack));
  text_layer_set_text_color(block_letter_layer_1, 
                            PBL_IF_COLOR_ELSE(gcolor_legible_over((GColor)(resistor_colors[block_values[1]])), GColorBlack));
  text_layer_set_text_color(block_letter_layer_2, 
                            PBL_IF_COLOR_ELSE(gcolor_legible_over((GColor)(resistor_colors[block_values[2]])), GColorBlack));

  text_layer_set_text(block_letter_layer_0, letter_0);
  text_layer_set_text(block_letter_layer_1, letter_1);
  text_layer_set_text(block_letter_layer_2, letter_2);
  
}

static void update_resistor_value_display(void){
  static char text_buffer[32], units[2], dot[2], zero[2];
  
  strcpy(dot, (block_values[2] % 3 == 2) ? "." : "\0");
  strcpy(zero, (block_values[2] % 3 == 1) ? "0" : "\0");
  
  if      (block_values[2] < 2) strcpy(units, "\0");
  else if (block_values[2] < 5) strcpy(units, "K");
  else                          strcpy(units, "M");
  
  if(block_values[0]){
    snprintf(text_buffer, sizeof(text_buffer), "%d%s%d%s%s\nohm", block_values[0], dot, block_values[1], zero, units);
  }
  else{
    snprintf(text_buffer, sizeof(text_buffer), "%d%s%s\nohm", block_values[1], zero, units);
  }
  
  text_layer_set_text(resistor_value_layer, text_buffer);
}

static void update_resistor_screen(void){
  layer_mark_dirty(resistor_color_layer);
  update_resistor_value_display();
  update_letters();
}

static void color_update_proc(Layer *layer, GContext *ctx){
  GRect bounds = layer_get_bounds(layer);
  GRect color_rects[3] = {
    GRect(0, BLOCK_TOP, bounds.size.w/3, bounds.size.w/3),
    GRect(bounds.size.w/NUM_RESISTOR_BLOCKS, BLOCK_TOP, bounds.size.w/3, bounds.size.w/3),
    GRect(2*bounds.size.w/NUM_RESISTOR_BLOCKS, BLOCK_TOP, bounds.size.w/3, bounds.size.w/3)
  };
  GRect value_rect = GRect(
    10,
    bounds.size.h/2 - 20,
    bounds.size.w - 20,
    bounds.size.h/2 + 15
  );
  
  //outer rect
  graphics_context_set_fill_color(ctx, RESISTOR_COLOR);
  graphics_fill_rect(ctx, GRect(0, 10, bounds.size.w, bounds.size.w/3), 0, GCornerNone);
  
  if(editing_color){
    graphics_context_set_fill_color(ctx, RESISTOR_SELECTOR_COLOR);
    graphics_fill_rect(ctx, color_rects[current_block], 0, GCornerNone);
  }
  else{
    graphics_context_set_fill_color(ctx, RESISTOR_SELECTOR_COLOR);
    graphics_fill_rect(ctx, value_rect, 0, GCornerNone);
    graphics_context_set_fill_color(ctx, RESISTOR_BACKGROUND_COLOR);
    graphics_fill_rect(ctx, grect_inset(value_rect, GEdgeInsets(RESISTOR_SELECTOR_WIDTH)), 0, GCornerNone);
  }
  
  for(int i = 0; i < NUM_RESISTOR_BLOCKS; i++){
    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE((GColor)(resistor_colors[block_values[i]]), GColorWhite));
    graphics_fill_rect(ctx, grect_inset(color_rects[i], GEdgeInsets(RESISTOR_SELECTOR_WIDTH)), 0, GCornerNone);
  }
  
}

static void resistor_up_single_click_handler(ClickRecognizerRef recognizer, void *context){
  if(editing_color){
    if (block_values[current_block] == block_value_max[current_block]) block_values[current_block] = 0;
    else block_values[current_block]++;
  }
  else{
    if(block_values[1] < block_value_max[1]){
      block_values[1]++;
    }
    else if(block_values[0] < block_value_max[0]){
      block_values[1] = block_value_min[1];
      block_values[0]++;
    }
    else if(block_values[2] < block_value_max[2]){
      block_values[1] = block_value_min[1];
      block_values[0] = block_value_min[0];
      block_values[2]++;
    }
  }
  
  update_resistor_screen();
}

static void resistor_select_single_click_handler(ClickRecognizerRef recognizer, void *context){
  if(editing_color){
    if (current_block == 2) current_block = 0;
    else current_block++;
  
    update_resistor_screen();
  }
}

static void resistor_down_single_click_handler(ClickRecognizerRef recognizer, void *context){ 
  if(editing_color){
    if (block_values[current_block] == 0) block_values[current_block] = block_value_max[current_block];
    else block_values[current_block]--;
  }
  else{
    if(block_values[1] > block_value_min[1]){
      block_values[1]--;
    }
    else if(block_values[0] > block_value_min[0]){
      block_values[1] = block_value_max[1];
      block_values[0]--;
    }
    else if(block_values[2] > block_value_min[2]){
      block_values[0] = block_value_max[0];
      block_values[1] = block_value_max[1];
      block_values[2]--;
    }
  }
  
  update_resistor_screen();
}

static void resistor_back_single_click_handler(ClickRecognizerRef recognizer, void *context){
  window_stack_pop(true);
}

static void resistor_select_long_click_handler(ClickRecognizerRef recognizer, void *context){
  editing_color = !editing_color;
  vibes_double_pulse();
  layer_mark_dirty(resistor_color_layer);
}

static void resistor_select_long_click_release_handler(ClickRecognizerRef recognizer, void *context){}

static void resistor_click_config_provider(Window *window){
  window_single_repeating_click_subscribe(BUTTON_ID_UP, RESISTOR_REPEATING_TIMEOUT, resistor_up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, resistor_select_single_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, RESISTOR_REPEATING_TIMEOUT, resistor_down_single_click_handler);
  
  window_single_click_subscribe(BUTTON_ID_BACK, resistor_back_single_click_handler);
  
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, resistor_select_long_click_handler, resistor_select_long_click_release_handler);
}

static void resistor_window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  GRect letter_rects[3] = {
    GRect(0, BLOCK_TOP + LETTER_VERTICAL_OFFSET, bounds.size.w/3, bounds.size.w/3),
    GRect(bounds.size.w/3, BLOCK_TOP + LETTER_VERTICAL_OFFSET, bounds.size.w/3, bounds.size.w/3),
    GRect(2*bounds.size.w/3, BLOCK_TOP + LETTER_VERTICAL_OFFSET, bounds.size.w/3, bounds.size.w/3)
  };
  
  window_set_click_config_provider(resistor_window, (ClickConfigProvider) resistor_click_config_provider);
  window_set_background_color(resistor_window, RESISTOR_BACKGROUND_COLOR);
  
  resistor_color_layer = layer_create(bounds);
  layer_set_update_proc(resistor_color_layer, color_update_proc);
  
  block_letter_layer_0 = text_layer_create(letter_rects[0]);
  block_letter_layer_1 = text_layer_create(letter_rects[1]);
  block_letter_layer_2 = text_layer_create(letter_rects[2]);
 
  text_layer_set_background_color(block_letter_layer_0, GColorClear);
  text_layer_set_background_color(block_letter_layer_1, GColorClear);
  text_layer_set_background_color(block_letter_layer_2, GColorClear);
  
  text_layer_set_text_alignment(block_letter_layer_0, GTextAlignmentCenter);
  text_layer_set_text_alignment(block_letter_layer_1, GTextAlignmentCenter);
  text_layer_set_text_alignment(block_letter_layer_2, GTextAlignmentCenter);
  
  text_layer_set_font(block_letter_layer_0, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(block_letter_layer_1, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(block_letter_layer_2, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  
  resistor_value_layer = text_layer_create(GRect(
    0,
    bounds.size.h/2 - 20,
    bounds.size.w,
    bounds.size.h/2 + 20
  ));
  
  text_layer_set_font(resistor_value_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(resistor_value_layer, GTextAlignmentCenter);
  text_layer_set_background_color(resistor_value_layer, GColorClear);
  update_resistor_screen();
  
  layer_add_child(window_layer, resistor_color_layer);
  layer_add_child(window_layer, text_layer_get_layer(block_letter_layer_0));
  layer_add_child(window_layer, text_layer_get_layer(block_letter_layer_1));
  layer_add_child(window_layer, text_layer_get_layer(block_letter_layer_2));
  layer_add_child(window_layer, text_layer_get_layer(resistor_value_layer));
  
}

static void resistor_window_unload(Window *window){
  layer_destroy(resistor_color_layer);
  text_layer_destroy(block_letter_layer_0);
  text_layer_destroy(block_letter_layer_1);
  text_layer_destroy(block_letter_layer_2);
  text_layer_destroy(resistor_value_layer);
}

static void resistor_init(void){
  resistor_window = window_create();
  window_set_window_handlers(resistor_window, (WindowHandlers){
    .load = resistor_window_load,
    .unload = resistor_window_unload
  });
 // window_stack_push(resistor_window, true);
}

static void resistor_deinit(void){
  window_destroy(resistor_window);
}