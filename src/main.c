#include <pebble.h>
#include "resistor.h"
#include "capacitor.h"

#if defined(PBL_COLOR)
  #define BACKGROUND_COLOR GColorCeleste
  #define HIGHLIGHT_COLOR GColorMediumSpringGreen
#else 
  #define BACKGROUND_COLOR GColorWhite
  #define HIGHLIGHT_COLOR GColorDarkGray
#endif

#define TEXT_HEIGHT 40

#define NUM_MENU_ITEMS 2

static Window *main_window;
static TextLayer *resistor_text_layer, *capacitor_text_layer;

static uint8_t menu_item = 0;

static void update_menu(void){
  switch(menu_item){
    case 0:
      text_layer_set_background_color(resistor_text_layer, HIGHLIGHT_COLOR);
      text_layer_set_background_color(capacitor_text_layer, GColorClear);
      break;
    case 1:
      text_layer_set_background_color(resistor_text_layer, GColorClear);
      text_layer_set_background_color(capacitor_text_layer, HIGHLIGHT_COLOR);
      break;
    default: break;
  }
}

static void main_up_single_click_handler(ClickRecognizerRef recognizer, void *context){
  if(menu_item > 0)
    menu_item--;
  
  update_menu();
}

static void main_select_single_click_handler(ClickRecognizerRef recognizer, void *context){
  switch(menu_item){
    case 0:
      window_stack_push(resistor_window, true);
      break;
    case 1:
      window_stack_push(capacitor_window, true);
      break;
    default: break;
  }
  
}

static void main_down_single_click_handler(ClickRecognizerRef recognizer, void *context){
  if(menu_item < NUM_MENU_ITEMS)
    menu_item++;
  
  update_menu();
}

static void config_provider(Window *window){
  window_single_click_subscribe(BUTTON_ID_UP, main_up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, main_select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, main_down_single_click_handler);
}

static void main_window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  GTextAlignment menu_item_alignment = GTextAlignmentLeft;
  GFont menu_item_font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  
  resistor_text_layer = text_layer_create(GRect(0, 0, bounds.size.w, TEXT_HEIGHT));
  
  text_layer_set_text_alignment(resistor_text_layer, menu_item_alignment);
  text_layer_set_font(resistor_text_layer, menu_item_font);
  text_layer_set_background_color(resistor_text_layer, GColorClear);
  text_layer_set_text(resistor_text_layer, "Resistor");
  
  capacitor_text_layer = text_layer_create(GRect(0, TEXT_HEIGHT, bounds.size.w, TEXT_HEIGHT));
  
  text_layer_set_text_alignment(capacitor_text_layer, menu_item_alignment);
  text_layer_set_font(capacitor_text_layer, menu_item_font);
  text_layer_set_background_color(capacitor_text_layer, GColorClear);
  text_layer_set_text(capacitor_text_layer, "Capacitor");
  
  layer_add_child(window_layer, text_layer_get_layer(resistor_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(capacitor_text_layer));
  
  update_menu();
}

static void main_window_unload(Window *window){
  text_layer_destroy(resistor_text_layer);
  text_layer_destroy(capacitor_text_layer);
}

static void init(void){
  resistor_init();
  capacitor_init();
  
  main_window = window_create();
  
  window_set_click_config_provider(main_window, (ClickConfigProvider)config_provider);
  
  window_set_window_handlers(main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_set_background_color(main_window, BACKGROUND_COLOR);
  window_stack_push(main_window, true);
}

static void deinit(void){
  resistor_deinit();
  capacitor_deinit();
  
  window_destroy(main_window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}
