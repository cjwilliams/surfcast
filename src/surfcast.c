// Create structures for data
// Plugin UI code
// Persist write
// Draw tide heights

#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include "app_msg.h"

#define MY_UUID { 0x55, 0xFF, 0x55, 0x62, 0x68, 0xBF, 0x4B, 0xF8, 0x83, 0xD4, 0x44, 0xE6, 0xB0, 0x9E, 0x0D, 0x59 }
PBL_APP_INFO( MY_UUID,
             "Pebble Surfcast", "Girlgrammer",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP );

Window *window;
TextLayer *text_layer;
BitmapLayer *icon_layer;

// static GBitmap icon_bitmap;

static void init( void ) {
	window = window_create();
  
  text_layer = text_layer_create( GRect( 0,60,144,108 ) );
  text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
  text_layer_set_text( text_layer, "Initialized" );
  layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );

	// create_splash_display();
	app_message_init();
	
	window_stack_push( window, true );
}

static void deinit( void ) {
	app_message_deinit();
	text_layer_destroy( text_layer );
  window_destroy( window );
}

int main( void ) {
  init();
	app_event_loop();
  deinit();
}
