#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>


#define MY_UUID { 0x50, 0x6A, 0x7E, 0x90, 0x0F, 0xCB, 0x4F, 0xD1, 0x91, 0x44, 0x01, 0xAE, 0x7F, 0x10, 0xC9, 0x15 }
PBL_APP_INFO( MY_UUID,
              "Surfcast", "Girlgrammer",
              1, 0, /* App version */
              DEFAULT_MENU_ICON,
              APP_INFO_STANDARD_APP );

#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168
#define STATUS_BAR_HEIGHT 16

static Window *window;
static TextLayer *box_layer;

// void select_click_handler(ClickRecognizerRef recognizer, void *context) {
//   text_layer_set_text(text_layer, "Select");
// }
// 
// void up_click_handler(ClickRecognizerRef recognizer, void *context) {
//   text_layer_set_text(text_layer, "Up");
// }
// 
// void down_click_handler(ClickRecognizerRef recognizer, void *context) {
//   text_layer_set_text(text_layer, "Down");
// }

// void config_provider(ClickConfig **config, Window *window) {
//   config[BUTTON_ID_SELECT]->click.handler = select_click_handler;
//   config[BUTTON_ID_UP]->click.handler = up_click_handler;
//   config[BUTTON_ID_DOWN]->click.handler = down_click_handler;
// }

void create_box( char *text, int width_units, int x, int y ) {
	box_layer = text_layer_create( GRect( x,y,( width_units * SCREEN_WIDTH )/2,SCREEN_HEIGHT/3 ) ) ;
	
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( box_layer ) );
	text_layer_set_text( box_layer, text );
	text_layer_set_text_alignment( box_layer, GTextAlignmentCenter );
}

void handle_init( void ) {
  window = window_create();
  window_stack_push( window, true /* Animated */ );
	
	// Forecast View
	create_box( "Overview",2,0,0 );
	create_box( "Swell",1,0,SCREEN_HEIGHT/3 );
	create_box( "Tide",1,SCREEN_WIDTH/2,SCREEN_HEIGHT/3 );
	create_box( "Wind",1,0,2*SCREEN_HEIGHT/3 );
	create_box( "Warning",1,SCREEN_WIDTH/2,2*SCREEN_HEIGHT/3 );
}

void handle_deinit( void ) {
	window_destroy( window );
}

int main( void ) {
  handle_init();
  app_event_loop();
  handle_deinit();
}