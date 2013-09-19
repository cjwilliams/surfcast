// TODO: Formalize passing of spot id's to phone and storage of results on watch
// Add tide API call w/request&response handling
// Plug into existing display code

#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include <time.h>
#include "defs.h"

#define MY_UUID { 0x55, 0xFF, 0x55, 0x62, 0x68, 0xBF, 0x4B, 0xF8, 0x83, 0xD4, 0x44, 0xE6, 0xB0, 0x9E, 0x0D, 0x59 }
PBL_APP_INFO( MY_UUID,
             "Comms App", "Cherie",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP );

Location locations[] = {
	{ 113, "Fort Point" },
	{ 649, "Eagles Point" },
	{ 697, "Kellys Cove" },
	{ 114, "North Ocean Beach" },
	{ 117, "South Ocean Beach" }
};

static Window *window;
static TextLayer *text_layer;
static BitmapLayer *icon_layer;

static GBitmap icon_bitmap;

// void init_forecasts( Forecast *forecast, int size ) {
// 	while( forecast++ < size ) {
// 		memset( forecast.time,0,sizeof( struct tm ) );
// 		forecast.overall = forecast.swell = forecast.tide = forecast.wind = forecast.swell_size = forecast.tide_height = forecast.isset = 0;
// 	}
// }
// 
// void set_forecast( Forecast *forecast ){ 
// 	// TODO
// 	forecast.isset = true;
// }
// 
// /* Need a tick(?) callback that calls expire_forecast on all old forecasts */
// void expire_forecast( Forecast *forecast ){
// 	forecast.expired = false;
// }

static void in_received_handler( DictionaryIterator *received, void *context ) {
	APP_LOG( APP_LOG_LEVEL_DEBUG, "Received data: "/*, data_tuple->value->cstring*/ );
	
	Tuple *data_tuple = dict_find( received, DATA_KEY );
}

static void in_dropped_handler( void *context, AppMessageResult reason ) {
	APP_LOG( APP_LOG_LEVEL_DEBUG, "Failed receiving data: "/*, data_tuple->value->cstring*/ );
}

static void out_sent_handler( DictionaryIterator *sent, void *context ) {
	APP_LOG( APP_LOG_LEVEL_DEBUG, "Sent data: "/*, data_tuple->value->cstring*/ );
}

static void out_failed_handler( DictionaryIterator *failed, AppMessageResult reason, void *context ) {
	APP_LOG( APP_LOG_LEVEL_DEBUG, "Failed sending data: "/*, data_tuple->value->cstring*/ );
}

static AppMessageCallbacksNode app_msg_callbacks = {
  .callbacks = {
		.in_received = in_received_handler,
		.in_dropped = in_dropped_handler,
		.out_sent = out_sent_handler,
		.out_failed = out_failed_handler,
  },
};

static void app_message_init( void ) {
  app_message_open( 64, 64 );
  app_message_register_callbacks( &app_msg_callbacks );

	DictionaryIterator *iter;
	
  if ( app_message_out_get( &iter ) != APP_MSG_OK ) {
    return;
  }
  // if ( dict_write_tuplet( iter, &location_tuplet ) != DICT_OK ) {
  if( dict_write_uint8( iter, LOCATION_KEY, 5 ) != DICT_OK || dict_write_uint8( iter, DURATION_KEY, 1 ) != DICT_OK ) {
  	return;
  }
  app_message_out_send();
  app_message_out_release();
}

static void app_message_deinit( void ) {
	app_message_deregister_callbacks( &app_msg_callbacks );
}

static void init( void ) {
	window = window_create();
  
  icon_layer = bitmap_layer_create( GRect( 32,10,80,80 ) );
	gbitmap_init_with_resource( &icon_bitmap, CONDITION_ICONS[ 0 ] ); 
	bitmap_layer_set_bitmap( icon_layer, &icon_bitmap ); /* Make sure to unload icons as icon_bitmap is reused */
  layer_add_child(window_get_root_layer( window ), bitmap_layer_get_layer( icon_layer ) );

  text_layer = text_layer_create( GRect( 0,100,144,68 ) );
  text_layer_set_background_color( text_layer, GColorClear );
  text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
  text_layer_set_text( text_layer, "Start" );
  layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );

  window_stack_push( window, true );

	// for( int i=0;locations[ i ] < NUM_LOCATIONS;i++ ) { 
	// 	init_forecasts( &locations[ i ], NUM_FORECASTS ); 
	// }
	
	app_message_init();
}

static void deinit( void ) {
	app_message_deinit();
	
  gbitmap_deinit( &icon_bitmap );
  
  text_layer_destroy( text_layer );
  bitmap_layer_destroy( icon_layer );
  window_destroy( window );
}

int main( void ) {
  init();
  app_event_loop();
  deinit();
}
