// On 9/26/13, switched to DP2 SDK which made a change to gbitmap init and deinit
//
// For future reference:
//
// OLD: bool gbitmap_init_with_resource(GBitmap* bitmap, int resource_id);
//			void gbitmap_deinit(GBitmap* bitmap);
// NEW: GBitmap* gbitmap_create_with_resource( int resource_id );
//			void gbitmap_destroy( GBitmap *bitmap );
// 	

#include <pebble.h>

#include "display.h"
#include "constants.h"
#include "forecast.h"
#include "utils.h"

// Icon resources for displaying forecast conditions
static const uint32_t CONDITION_ICONS[] = {
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

static Location *drawable_location;	// Used to pass location between menu_row and forecast display (location param not supported in callbacks)

static int current_title_layer = 0;	// Index for forecast_title_layers
static int current_value_layer = 0;	// Index for forecast_value_layers
static int current_icon_layer = 0;		// Index for forecast_icon_layers

//========== Forecast ==========
static char *getDescription( int value ) {
	switch( value ) {
		case 0: return "Poor"; break;
		case 1: return "Poor-Fair"; break;
		case 2: return "Fair"; break;
		case 3: return "Fair-Good"; break;
		case 4: return "Good"; break;
		default: return "Error"; break;
	}
}	// Returns text description associated with numeric forecast condition

// create_box creates a box of full or half screen width and 1/3 screen height, with a description, condition icon, and condition text
static void create_box( char *text, int width_units, int x, int y, int value ) {
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

// create_text_box creates a box of full or half screen width and 1/3 screen height, with a description and large text value
static void create_text_box( char *text, int width_units, int x, int y, char *value ) {
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

// create_error_box displays a centered error message on a full screen
static void display_forecast_error( void ){
	TextLayer *text_layer;
	
	text_layer = text_layer_create( GRect( 0,USABLE_HEIGHT/3,SCREEN_WIDTH,USABLE_HEIGHT/3 ) );
	text_layer_set_text( text_layer, "There are no current forecasts for this location" );
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_background_color( text_layer, GColorClear);
	layer_add_child( window_get_root_layer( forecast_window ), text_layer_get_layer( text_layer ) );
	if( current_title_layer < MAX_LAYERS ){ forecast_title_layers[ current_title_layer++ ] = text_layer; }
}

static void forecast_load( Window *window ) {
	if( has_current_forecast( drawable_location ) ){
		create_box( drawable_location->name, 2, 0, 0, get_current_conditions( drawable_location, OVERALL ) );
		create_box( "Swell", 1, 0, USABLE_HEIGHT/3, get_current_conditions( drawable_location, SWELL ) );
		create_box( "Tide", 1, SCREEN_WIDTH/2, USABLE_HEIGHT/3, get_current_conditions( drawable_location, TIDE ) );
		create_box( "Wind", 1, 0, 2*USABLE_HEIGHT/3, get_current_conditions( drawable_location, WIND ) );
		create_text_box( "Size", 1, SCREEN_WIDTH/2, 2*USABLE_HEIGHT/3, get_current_swell_size( drawable_location ) );
	}
	else{
		display_forecast_error();
	}
}

static void forecast_unload( Window *window ) {
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
	window_destroy( forecast_window );
}

static void create_forecast_display() {
	forecast_window = window_create();
	
	window_set_window_handlers( forecast_window, (WindowHandlers){
    .load = forecast_load,
    .unload = forecast_unload,
  });

	window_stack_push( forecast_window, true );
}

//========== Locations Menu ==========
void update_menu( void ){
	if( menu_layer != NULL ){
		menu_layer_reload_data( menu_layer );
		layer_mark_dirty( menu_layer_get_layer( menu_layer ) );
	}
}	// Updates menu with new condition icon and scrolling layer height, as applicable

static uint16_t menu_get_num_rows_callback( MenuLayer *menu_layer, uint16_t section_index, void *data ) {
  return get_num_locations();
}

static void menu_draw_row_callback( GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data ) {
	Location *location = get_location_by_index( cell_index->row );
	if( location != NULL && has_current_forecast( location ) ){
		menu_cell_basic_draw( ctx, cell_layer, get_spot_name( location ), get_county_name( location ), condition_icons[ get_current_conditions( location, OVERALL ) ] );
	}
	else{
		APP_LOG( APP_LOG_LEVEL_WARNING, "Attempted to access a NULL location" );	// This shouldn't happen anymore
	}
}

static void menu_select_callback( MenuLayer *menu_layer, MenuIndex *cell_index, void *data ) {
	drawable_location = get_location_by_index( cell_index->row );	// Set global drawable_location to be accessed from forecast window
	if( drawable_location != NULL ){
		create_forecast_display();
	}
	else{
		APP_LOG( APP_LOG_LEVEL_WARNING, "Attempted to access a NULL location" );	// This shouldn't happen anymore
	}
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

	window_destroy( menu_window );
}

static void create_menu_display( void ) {
	menu_window = window_create();
	
	window_set_window_handlers( menu_window, (WindowHandlers){
    .load = menu_load,
    .unload = menu_unload,
  });
	
	window_stack_push( menu_window, true );
}

//========== Splash ==========
static void splash_click_handler( ClickRecognizerRef recognizer, void *context ) {
	create_menu_display();
}

static void splash_config_provider( Window *window ) {
	window_single_click_subscribe( BUTTON_ID_UP, splash_click_handler );
	window_single_click_subscribe( BUTTON_ID_SELECT, splash_click_handler );
	window_single_click_subscribe( BUTTON_ID_DOWN, splash_click_handler );
	// Makes all clicks direct to menu screen
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
	
	splash_credits_layer = text_layer_create( GRect( 0,USABLE_HEIGHT-STATUS_BAR_HEIGHT,SCREEN_WIDTH,STATUS_BAR_HEIGHT*2 ) );
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
	window_destroy( splash_window );
}

static void create_splash_display( void ) {	
	splash_window = window_create();
  
	window_set_window_handlers( splash_window, (WindowHandlers){
    .load = splash_load,
    .unload = splash_unload,
  });

	window_set_background_color( splash_window, GColorBlack );
	window_set_fullscreen( splash_window, true );
	window_stack_push( splash_window, true );
}

//========== Base ==========
void init_windows( void ){
	create_splash_display();
}

void deinit_windows( void ){
	window_stack_pop_all( true );
}