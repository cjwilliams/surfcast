#include <stdlib.h>
#include <string.h>

#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include "forecast.h"

#define MY_UUID { 0x50, 0x6A, 0x7E, 0x90, 0x0F, 0xCB, 0x4F, 0xD1, 0x91, 0x44, 0x01, 0xAE, 0x7F, 0x10, 0xC9, 0x15 }
PBL_APP_INFO( MY_UUID,
              "Surfcast", "Girlgrammer",
              1, 0, /* App version */
              DEFAULT_MENU_ICON,
              APP_INFO_STANDARD_APP );

#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168
#define STATUS_BAR_HEIGHT 16
#define USABLE_HEIGHT (( SCREEN_HEIGHT )-( STATUS_BAR_HEIGHT ))

#define NUM_RATINGS 5
#define NUM_SPOTS 5

static Window *window;
static TextLayer *text_layer;
static MenuLayer *menu_layer;
static BitmapLayer *icon_layer;

Forecast forecasts[5];

static GBitmap splash_bitmap, icon_bitmap;

static AppSync sync;
static uint8_t sync_buffer[32];

enum ForecastKeys {
  LOCATION_KEY = 0x0,				// TUPLE_CSTRING
	TIME_KEY = 0x1,
  OVERALL_KEY = 0x2,  					// TUPLE_BYTE_ARRAY
	SWELL_KEY = 0x3,
	TIDE_KEY = 0x4,
	WIND_KEY = 0x5,
	SIZE_KEY = 0x6,
};

static uint32_t RATING_ICONS[] = {
  RESOURCE_ID_POOR_COND,
  RESOURCE_ID_PF_COND,
  RESOURCE_ID_FAIR_COND,
  RESOURCE_ID_FG_COND,
	RESOURCE_ID_GOOD_COND
};

void create_menu_screen( void );
void create_dashboard( Forecast *forecast );

/********** SPLASH **********/
void splash_click_handler( ClickRecognizerRef recognizer, void *context ) {
	create_menu_screen();
}

void splash_config_provider( ClickConfig **config, Window *window ) {
  config[ BUTTON_ID_SELECT ]->click.handler = config[ BUTTON_ID_UP ]->click.handler = config[ BUTTON_ID_DOWN ]->click.handler  = splash_click_handler;
}

void create_splash_screen( void ) {
	window = window_create();
  window_set_background_color( window, GColorBlack );
	window_stack_push( window, true /* Animated */ );
	
	text_layer = text_layer_create( GRect( 0,0,SCREEN_WIDTH,USABLE_HEIGHT/2 ) );
	text_layer_set_text_color( text_layer, GColorWhite);
  text_layer_set_background_color( text_layer, GColorClear);
	text_layer_set_text( text_layer, "Pebble Surfcast" );
	text_layer_set_font( text_layer, fonts_get_system_font( FONT_KEY_GOTHIC_28_BOLD ) );
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_overflow_mode( text_layer, GTextOverflowModeWordWrap );
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );
	
	icon_layer = bitmap_layer_create( GRect( 0, USABLE_HEIGHT/2, SCREEN_WIDTH, 40 ) );
  gbitmap_init_with_resource( &splash_bitmap, RESOURCE_ID_SPLASH_ICON );
	bitmap_layer_set_bitmap( icon_layer, &splash_bitmap );
  layer_add_child( window_get_root_layer( window ), bitmap_layer_get_layer( icon_layer ) );
	
	text_layer = text_layer_create( GRect( 0,USABLE_HEIGHT-( 2*STATUS_BAR_HEIGHT ),SCREEN_WIDTH,STATUS_BAR_HEIGHT*2 ) );
	text_layer_set_text_color( text_layer, GColorWhite);
  text_layer_set_background_color( text_layer, GColorClear);
	text_layer_set_text( text_layer, "Data provided by Spitcast (www.spitcast.com)" );
	text_layer_set_font( text_layer, fonts_get_system_font( FONT_KEY_GOTHIC_14 ) );
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_overflow_mode( text_layer, GTextOverflowModeWordWrap );
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );
	
	window_set_click_config_provider( window, ( ClickConfigProvider ) splash_config_provider );
}

void destroy_splash_screen( void ) {
	layer_destroy( window_get_root_layer( window ) );
	bitmap_layer_destroy( icon_layer );
	gbitmap_deinit( &splash_bitmap );
	window_destroy( window );
}

/********** LOCATIONS MENU **********/
static uint16_t menu_get_num_rows_callback( MenuLayer *menu_layer, uint16_t section_index, void *data ) {
  return NUM_SPOTS;
}

static void menu_draw_row_callback( GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data ) {
	for( int i=0;i<5;i++ ) {
		gbitmap_init_with_resource( &icon_bitmap, RATING_ICONS[ 0 ] );
		menu_cell_basic_draw( ctx,cell_layer,"Spot 1",NULL,&icon_bitmap );
		gbitmap_deinit( &icon_bitmap );
	}
}

void menu_select_callback( MenuLayer *menu_layer, MenuIndex *cell_index, void *data ) {
	create_dashboard( &forecasts[ cell_index->row ] );
}

void menu_load ( Window *window ) {	
	Layer *window_layer = window_get_root_layer( window );
	menu_layer = menu_layer_create( layer_get_frame( window_layer ) );
	
	menu_layer_set_callbacks( menu_layer, NULL, ( MenuLayerCallbacks ){
    .get_num_rows = menu_get_num_rows_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

	menu_layer_set_click_config_onto_window( menu_layer, window );
	layer_add_child( window_layer, menu_layer_get_layer( menu_layer ) );
}

void menu_unload( Window *window ) {
  menu_layer_destroy( menu_layer );
}

void create_menu_screen( void ) {
	window = window_create();
	
	window_set_window_handlers( window, ( WindowHandlers ) {
    .load = menu_load,
    .unload = menu_unload,
  });
	
	window_stack_push( window, true );
}

/********** FORECAST **********/
char *getDescription( int value ) {
	switch( value ) {
		case 0: return "Poor"; break;
		case 1: return "Poor-Fair"; break;
		case 2: return "Fair"; break;
		case 3: return "Fair-Good"; break;
		case 4: return "Good"; break;
		default: return "Error"; break;
	}
}

void create_box( char *text, int width_units, int x, int y, int value ) {
	icon_layer = bitmap_layer_create( GRect( x,y,( width_units * SCREEN_WIDTH )/2,USABLE_HEIGHT/3 ) );
	gbitmap_init_with_resource( &icon_bitmap, RATING_ICONS[ value ]);
  bitmap_layer_set_bitmap( icon_layer, &icon_bitmap );
  layer_add_child( window_get_root_layer( window ), bitmap_layer_get_layer( icon_layer ) );

	text_layer = text_layer_create( GRect( x,y,( width_units * SCREEN_WIDTH )/2,USABLE_HEIGHT/3 ) );
	text_layer_set_text( text_layer, text );
	
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_background_color( text_layer, GColorClear);
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );
	
	text_layer = text_layer_create( GRect( x,y+( ( USABLE_HEIGHT/3 )-STATUS_BAR_HEIGHT ),( width_units * SCREEN_WIDTH )/2,USABLE_HEIGHT/3 ) );
	text_layer_set_text( text_layer, getDescription( value ) );
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_font( text_layer, fonts_get_system_font( FONT_KEY_GOTHIC_14 ) );
	text_layer_set_background_color( text_layer, GColorClear);
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );
}

void create_dashboard( Forecast *forecast ) {
	window = window_create();
	window_stack_push( window, true /* Animated */ );
	
	create_box( forecast->location,2,0,0, forecast->overall );
	create_box( "Swell",1,0,USABLE_HEIGHT/3, forecast->swell );
	create_box( "Tide",1,SCREEN_WIDTH/2,USABLE_HEIGHT/3, forecast->tide );
	create_box( "Wind",1,0,2*USABLE_HEIGHT/3, forecast->wind );
	create_box( "Size",1,SCREEN_WIDTH/2,2*USABLE_HEIGHT/3, forecast->size );
}

/********** MAIN APP LOOP **********/
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case LOCATION_KEY:
    	text_layer_set_text( text_layer, new_tuple->value->cstring );
			break;

    case OVERALL_KEY:
      // App Sync keeps the new_tuple around, so we may use it directly
			gbitmap_deinit( &icon_bitmap );
			gbitmap_init_with_resource( &icon_bitmap, RATING_ICONS[ new_tuple->value->uint8 ]);
      break;
  }
}

static void send_cmd() {
  Tuplet value = TupletInteger( 1,1 );

  DictionaryIterator *iter;
  app_message_out_get( &iter );

  if ( iter == NULL ) {
    return;
  }

  dict_write_tuplet( iter, &value );
  dict_write_end( iter );

  app_message_out_send();
  app_message_out_release();
}

void handle_init( void ) {
	app_message_open(64, 64);
	
	uint8_t byte_array[] = { 3,3,2,4,2 };
	Tuplet initial_values[] = {
		TupletCString( LOCATION_KEY, "Pebble Beach" ),
		TupletCString( TIME_KEY, "2013-09-16 1" ),
		TupletInteger( OVERALL_KEY, 4 ),
//    TupletBytes( DATA_KEY, byte_array, 5 /* Length */ )
  };

	for(int i=0;i<5;i++){ forecasts[i].overall = forecasts[i].swell = forecasts[i].tide = forecasts[i].wind = 4; forecasts[i].location="Location"; forecasts[i].size=2; }

  // send_cmd();

  app_sync_init( &sync, sync_buffer, sizeof( sync_buffer ), initial_values, ARRAY_LENGTH( initial_values ), sync_tuple_changed_callback, sync_error_callback, NULL );
	
	// create_splash_screen();
}

void handle_deinit( void ) {
	destroy_splash_screen();
}

int main( void ) {
	handle_init();
  app_event_loop();
	handle_deinit();
}