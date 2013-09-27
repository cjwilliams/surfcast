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

#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168
#define STATUS_BAR_HEIGHT 16
#define USABLE_HEIGHT (( SCREEN_HEIGHT )-( STATUS_BAR_HEIGHT ))

const uint32_t CONDITION_ICONS[] = {
  RESOURCE_ID_POOR_COND,
  RESOURCE_ID_PF_COND,
  RESOURCE_ID_FAIR_COND,
  RESOURCE_ID_FG_COND,
	RESOURCE_ID_GOOD_COND
};

Window *window;
TextLayer *text_layer;

static MenuLayer *menu_layer;
static TextLayer *title_layer;
static BitmapLayer *logo_layer;
static BitmapLayer *icon_layer;

static GBitmap *logo_bitmap;
static GBitmap *condition_icons[ NUM_CONDITIONS ];

Location *drawable_location;

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
	bitmap_layer_set_bitmap( icon_layer, condition_icons[ value ] );
	// unload: called when the window is deinited, but could be used in the future to free resources bound to windows that are not on screen.
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

void create_text_box( char *text, int width_units, int x, int y, char *value ) {
	text_layer = text_layer_create( GRect( x,y,( width_units * SCREEN_WIDTH )/2,USABLE_HEIGHT/3 ) );
	text_layer_set_text( text_layer, text );
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_background_color( text_layer, GColorClear);
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );
	
	text_layer = text_layer_create( GRect( x,y+( ( ( USABLE_HEIGHT/3 )-STATUS_BAR_HEIGHT )/2 ),( width_units * SCREEN_WIDTH )/2,USABLE_HEIGHT/3 ) );
	text_layer_set_text( text_layer, value );
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_font( text_layer, fonts_get_system_font( FONT_KEY_GOTHIC_14 ) );
	text_layer_set_background_color( text_layer, GColorClear);
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );
}


void forecast_load( Window *window ) {	
	create_box( drawable_location->name,2,0,0, get_current_conditions( drawable_location, OVERALL ) );
	create_box( "Swell",1,0,USABLE_HEIGHT/3, get_current_conditions( drawable_location, SWELL ) );
	create_box( "Tide",1,SCREEN_WIDTH/2,USABLE_HEIGHT/3, get_current_conditions( drawable_location, TIDE ) );
	create_box( "Wind",1,0,2*USABLE_HEIGHT/3, get_current_conditions( drawable_location, WIND ) );
	create_text_box( "Size",1,SCREEN_WIDTH/2,2*USABLE_HEIGHT/3, get_current_swell_size( drawable_location ) );
}

void forecast_unload( Window *window ) {
	layer_destroy( window_get_root_layer( window ) ); // Double check this is sufficient
}

void create_forecast_display() {
	window = window_create();
	
	window_set_window_handlers( window, (WindowHandlers){
    .load = forecast_load,
    .unload = forecast_unload,
  });

	window_stack_push( window, true );
}

/********** LOCATIONS MENU **********/
static uint16_t menu_get_num_rows_callback( MenuLayer *menu_layer, uint16_t section_index, void *data ) {
  return NUM_SPOTS;
}

static void menu_draw_row_callback( GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data ) {
	Location *location = get_location_by_index( cell_index->row );
	menu_cell_basic_draw( ctx, cell_layer, get_spot_name( location ), get_county( location ), condition_icons[ get_current_conditions( location, OVERALL ) ] );
}

static void menu_select_callback( MenuLayer *menu_layer, MenuIndex *cell_index, void *data ) {
	drawable_location = get_location_by_index( cell_index->row );
	create_forecast_display();
}

static void menu_load ( Window *window ) {	
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
	window = window_create();
	
	window_set_window_handlers( window, (WindowHandlers){
    .load = menu_load,
    .unload = menu_unload,
  });
	
	window_stack_push( window, true );
}

/********** SPLASH **********/
static void splash_click_handler( ClickRecognizerRef recognizer, void *context ) {
	create_menu_display();
}

static void splash_config_provider( ClickConfig **config, Window *window ) {
  config[ BUTTON_ID_SELECT ]->click.handler = config[ BUTTON_ID_UP ]->click.handler = config[ BUTTON_ID_DOWN ]->click.handler  = splash_click_handler;
}

static void splash_load( Window *window ) {	
	title_layer = text_layer_create( GRect( 0,0,SCREEN_WIDTH,USABLE_HEIGHT/2 ) );
	text_layer_set_text_color( title_layer, GColorWhite);
  text_layer_set_background_color( title_layer, GColorClear);
	text_layer_set_text( title_layer, "Pebble Surfcast" );
	text_layer_set_font( title_layer, fonts_get_system_font( FONT_KEY_GOTHIC_28_BOLD ) );
	text_layer_set_text_alignment( title_layer, GTextAlignmentCenter );
	text_layer_set_overflow_mode( title_layer, GTextOverflowModeWordWrap );
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( title_layer ) );
	
	logo_layer = bitmap_layer_create( GRect( 0, USABLE_HEIGHT/2, SCREEN_WIDTH, 40 ) );
  logo_bitmap = gbitmap_create_with_resource( RESOURCE_ID_SPLASH_ICON );
	bitmap_layer_set_bitmap( logo_layer, logo_bitmap );
  layer_add_child( window_get_root_layer( window ), bitmap_layer_get_layer( logo_layer ) );
	
	text_layer = text_layer_create( GRect( 0,USABLE_HEIGHT-( 2*STATUS_BAR_HEIGHT ),SCREEN_WIDTH,STATUS_BAR_HEIGHT*2 ) );
	text_layer_set_text_color( text_layer, GColorWhite);
  text_layer_set_background_color( text_layer, GColorClear);
	text_layer_set_text( text_layer, "Data provided by Spitcast (www.spitcast.com)" );
	text_layer_set_font( text_layer, fonts_get_system_font( FONT_KEY_GOTHIC_14 ) );
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_overflow_mode( text_layer, GTextOverflowModeWordWrap );
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );
	
	window_set_click_config_provider( window, (ClickConfigProvider) splash_config_provider );
}

static void splash_unload( Window *window ) {
	layer_destroy( window_get_root_layer( window ) );
	gbitmap_destroy( logo_bitmap );
}

static void create_splash_display( void ) {	
	window = window_create();
  
	window_set_window_handlers( window, (WindowHandlers){
    .load = splash_load,
    .unload = splash_unload,
  });

	window_set_background_color( window, GColorBlack );
	window_stack_push( window, true );
}

/********** BASE **********/
void init_windows( void ){
	create_splash_display();
}

void deinit_windows( void ){
}