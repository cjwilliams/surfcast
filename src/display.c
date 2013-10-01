// On 9/26/13, switched to DP2 SDK which made a change to gbitmap init and deinit
//
// For future reference:
//
// OLD: bool gbitmap_init_with_resource(GBitmap* bitmap, int resource_id);
//			void gbitmap_deinit(GBitmap* bitmap);
// NEW: GBitmap* gbitmap_create_with_resource( int resource_id );
//			void gbitmap_destroy( GBitmap *bitmap );
// 	

#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include "display.h"
#include "constants.h"
#include "forecast.h"
#include "utils.h"

const uint32_t CONDITION_ICONS[] = {
  RESOURCE_ID_POOR_COND,
  RESOURCE_ID_PF_COND,
  RESOURCE_ID_FAIR_COND,
  RESOURCE_ID_FG_COND,
	RESOURCE_ID_GOOD_COND
};

static Window *splash_window = NULL;
static Window *menu_window = NULL;
static Window *forecast_window = NULL;
static TextLayer *splash_title_layer = NULL;
static TextLayer *splash_credits_layer = NULL;
static TextLayer *forecast_title_layers[ MAX_LAYERS ] = { NULL };
static TextLayer *forecast_value_layers[ MAX_LAYERS ] = { NULL };
static MenuLayer *menu_layer = NULL;
static BitmapLayer *splash_logo_layer = NULL;
static BitmapLayer *forecast_icon_layers[ MAX_LAYERS ] = { NULL }; 

static GBitmap *splash_logo_bitmap;
static GBitmap *condition_icons[ NUM_CONDITIONS ];

Location *drawable_location;

int current_title_layer = 0;
int current_value_layer = 0;
int current_icon_layer = 0;

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
	BitmapLayer *icon_layer;
	TextLayer *text_layer;
	
	// icon_layer is listed before text_layer so text is overlayed on top of icon, not vice-versa
	icon_layer = bitmap_layer_create( GRect( x,y,( width_units * SCREEN_WIDTH )/2,USABLE_HEIGHT/3 ) );
	bitmap_layer_set_bitmap( icon_layer, condition_icons[ value ] );
	layer_add_child( window_get_root_layer( forecast_window ), bitmap_layer_get_layer( icon_layer ) );
	if( current_icon_layer < MAX_LAYERS ){ forecast_icon_layers[ current_icon_layer++ ] = icon_layer; }

	text_layer = text_layer_create( GRect( x,y,( width_units * SCREEN_WIDTH )/2,USABLE_HEIGHT/3 ) );
	text_layer_set_text( text_layer, text );	
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_background_color( text_layer, GColorClear);
	layer_add_child( window_get_root_layer( forecast_window ), text_layer_get_layer( text_layer ) );
	if( current_title_layer < MAX_LAYERS ){ forecast_title_layers[ current_title_layer++ ] = text_layer; }
	
	text_layer = text_layer_create( GRect( x,y+( ( USABLE_HEIGHT/3 )-STATUS_BAR_HEIGHT ),( width_units * SCREEN_WIDTH )/2,USABLE_HEIGHT/3 ) );
	text_layer_set_text( text_layer, getDescription( value ) );
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_font( text_layer, fonts_get_system_font( FONT_KEY_GOTHIC_14 ) );
	text_layer_set_background_color( text_layer, GColorClear);
	layer_add_child( window_get_root_layer( forecast_window ), text_layer_get_layer( text_layer ) );
	if( current_value_layer < MAX_LAYERS ){ forecast_value_layers[ current_value_layer++ ] = text_layer; }
}

void create_text_box( char *text, int width_units, int x, int y, char *value ) {
	TextLayer *text_layer;
	
	text_layer = text_layer_create( GRect( x,y,( width_units * SCREEN_WIDTH )/2,USABLE_HEIGHT/3 ) );
	text_layer_set_text( text_layer, text );
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_background_color( text_layer, GColorClear);
	layer_add_child( window_get_root_layer( forecast_window ), text_layer_get_layer( text_layer ) );
	if( current_title_layer < MAX_LAYERS ){ forecast_title_layers[ current_title_layer++ ] = text_layer; }
	
	text_layer = text_layer_create( GRect( x,y+( ( ( USABLE_HEIGHT/3 )-STATUS_BAR_HEIGHT )/2 ),( width_units * SCREEN_WIDTH )/2,USABLE_HEIGHT/3 ) );
	text_layer_set_text( text_layer, value );
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_font( text_layer, fonts_get_system_font( FONT_KEY_GOTHIC_14 ) );
	text_layer_set_background_color( text_layer, GColorClear);
	layer_add_child( window_get_root_layer( forecast_window ), text_layer_get_layer( text_layer ) );
	if( current_value_layer < MAX_LAYERS ){ forecast_value_layers[ current_value_layer++ ] = text_layer; }
}


void forecast_load( Window *window ) {	
	create_box( drawable_location->name, 2, 0, 0, get_current_conditions( drawable_location, OVERALL ) );
	create_box( "Swell", 1, 0, USABLE_HEIGHT/3, get_current_conditions( drawable_location, SWELL ) );
	create_box( "Tide", 1, SCREEN_WIDTH/2, USABLE_HEIGHT/3, get_current_conditions( drawable_location, TIDE ) );
	create_box( "Wind", 1, 0, 2*USABLE_HEIGHT/3, get_current_conditions( drawable_location, WIND ) );
	create_text_box( "Size", 1, SCREEN_WIDTH/2, 2*USABLE_HEIGHT/3, get_current_swell_size( drawable_location ) );
}

void forecast_unload( Window *window ) {
	for( int i=0; i<current_title_layer; i++ ){
		text_layer_destroy( forecast_title_layers[ i ] );
	}
	
	for( int j=0; j<current_icon_layer; j++ ){
		bitmap_layer_destroy( forecast_icon_layers[ j ] );
	}
	
	for( int k=0; k<current_value_layer; k++ ){
		text_layer_destroy( forecast_value_layers[ k ] );
	}
	
	current_title_layer = current_icon_layer = current_value_layer = 0;
}

void create_forecast_display() {
	forecast_window = window_create();
	
	window_set_window_handlers( forecast_window, (WindowHandlers){
    .load = forecast_load,
    .unload = forecast_unload,
  });

	window_stack_push( forecast_window, true );
}

/********** LOCATIONS MENU **********/
static uint16_t menu_get_num_rows_callback( MenuLayer *menu_layer, uint16_t section_index, void *data ) {
  return get_num_locations();
}

static void menu_draw_row_callback( GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data ) {
	Location *location = get_location_by_index( cell_index->row );
	menu_cell_basic_draw( ctx, cell_layer, get_spot_name( location ), get_county( location ), condition_icons[ get_current_conditions( location, OVERALL ) ] );
}

static void menu_select_callback( MenuLayer *menu_layer, MenuIndex *cell_index, void *data ) {
	set_current_datetime();
	drawable_location = get_location_by_index( cell_index->row );
	create_forecast_display();
}

static void menu_load ( Window *window ) {	
	set_current_datetime();
	
	for (int i = 0; i < NUM_CONDITIONS; i++) {
		condition_icons[ i ] = gbitmap_create_with_resource( CONDITION_ICONS[ i ] );
	}

	menu_layer = menu_layer_create( layer_get_frame( window_get_root_layer( window ) ) );
	
	menu_layer_set_callbacks( menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_rows = menu_get_num_rows_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

	menu_layer_set_click_config_onto_window( menu_layer, window );
	layer_add_child( window_get_root_layer( window ), menu_layer_get_layer( menu_layer ) );
}

static void menu_unload( Window *window ) {
  menu_layer_destroy( menu_layer );

	for (int i = 0; i < NUM_SPOTS; i++) {
    gbitmap_destroy( condition_icons[ i ] );
  }
}

static void create_menu_display( void ) {
	menu_window = window_create();
	
	window_set_window_handlers( menu_window, (WindowHandlers){
    .load = menu_load,
    .unload = menu_unload,
  });
	
	window_stack_push( menu_window, true );
}

/********** SPLASH **********/
static void splash_click_handler( ClickRecognizerRef recognizer, void *context ) {
	create_menu_display();
}

static void splash_config_provider( ClickConfig **config, Window *window ) {
  config[ BUTTON_ID_SELECT ]->click.handler = config[ BUTTON_ID_UP ]->click.handler = config[ BUTTON_ID_DOWN ]->click.handler  = splash_click_handler;
}

static void splash_load( Window *window ) {	
	splash_title_layer = text_layer_create( GRect( 0,0,SCREEN_WIDTH,USABLE_HEIGHT/2 ) );
	text_layer_set_text_color( splash_title_layer, GColorWhite);
  text_layer_set_background_color( splash_title_layer, GColorClear);
	text_layer_set_text( splash_title_layer, "Pebble Surfcast" );
	text_layer_set_font( splash_title_layer, fonts_get_system_font( FONT_KEY_GOTHIC_28_BOLD ) );
	text_layer_set_text_alignment( splash_title_layer, GTextAlignmentCenter );
	text_layer_set_overflow_mode( splash_title_layer, GTextOverflowModeWordWrap );
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( splash_title_layer ) );
	
	splash_logo_bitmap = gbitmap_create_with_resource( RESOURCE_ID_SPLASH_ICON );
	splash_logo_layer = bitmap_layer_create( GRect( 0, USABLE_HEIGHT/2, SCREEN_WIDTH, 40 ) );
  bitmap_layer_set_bitmap( splash_logo_layer, splash_logo_bitmap );
  layer_add_child( window_get_root_layer( window ), bitmap_layer_get_layer( splash_logo_layer ) );
	
	splash_credits_layer = text_layer_create( GRect( 0,USABLE_HEIGHT-( 2*STATUS_BAR_HEIGHT ),SCREEN_WIDTH,STATUS_BAR_HEIGHT*2 ) );
	text_layer_set_text_color( splash_credits_layer, GColorWhite);
  text_layer_set_background_color( splash_credits_layer, GColorClear);
	text_layer_set_text( splash_credits_layer, "Data provided by Spitcast (www.spitcast.com)" );
	text_layer_set_font( splash_credits_layer, fonts_get_system_font( FONT_KEY_GOTHIC_14 ) );
	text_layer_set_text_alignment( splash_credits_layer, GTextAlignmentCenter );
	text_layer_set_overflow_mode( splash_credits_layer, GTextOverflowModeWordWrap );
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( splash_credits_layer ) );
	
	window_set_click_config_provider( window, (ClickConfigProvider) splash_config_provider );
}

static void splash_unload( Window *window ) {
	text_layer_destroy( splash_title_layer );
	gbitmap_destroy( splash_logo_bitmap );
	bitmap_layer_destroy( splash_logo_layer );
	text_layer_destroy( splash_credits_layer );
}

static void create_splash_display( void ) {	
	splash_window = window_create();
  
	window_set_window_handlers( splash_window, (WindowHandlers){
    .load = splash_load,
    .unload = splash_unload,
  });

	window_set_background_color( splash_window, GColorBlack );
	window_stack_push( splash_window, true );
}

/********** BASE **********/
void init_windows( void ){
	create_splash_display();
}

void deinit_windows( void ){
	window_stack_pop_all( true );
}