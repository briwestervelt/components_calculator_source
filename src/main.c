#include <pebble.h>
#include "resistor.h"

static void init(void){
  resistor_init();
}

static void deinit(void){
  resistor_deinit();
}

int main(void){
  init();
  app_event_loop();
  deinit();
}
