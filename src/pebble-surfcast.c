#include <stdlib.h>
#include <string.h>

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
#define USABLE_HEIGHT (( SCREEN_HEIGHT )-( STATUS_BAR_HEIGHT ))

#define NUM_RATINGS 5
#define NUM_SPOTS 5

static Window *window;
static TextLayer *text_layer;
static MenuLayer *menu_layer;
static BitmapLayer *icon_layer;
static GBitmap splash_bitmap;

static GBitmap rating_icons[ NUM_RATINGS ];

typedef struct {
	char *location;
	int overall;
	int swell;
	int tide;
	int wind;
	int warning;
} Forecast;

static Forecast *forecast;

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
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );
	text_layer_set_text( text_layer, "Pebble Surfcast" );
	text_layer_set_font( text_layer, fonts_get_system_font( FONT_KEY_GOTHIC_28_BOLD ) );
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_overflow_mode( text_layer, GTextOverflowModeWordWrap );
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );
	
	icon_layer = bitmap_layer_create(GRect(0, USABLE_HEIGHT/2, SCREEN_WIDTH, 40));
  bitmap_layer_set_bitmap( icon_layer, &splash_bitmap );
  layer_add_child(window_get_root_layer( window ), bitmap_layer_get_layer( icon_layer ) );
	
	text_layer = text_layer_create( GRect( 0,USABLE_HEIGHT-( 2*STATUS_BAR_HEIGHT ),SCREEN_WIDTH,STATUS_BAR_HEIGHT*2 ) );
	text_layer_set_text_color( text_layer, GColorWhite);
  text_layer_set_background_color( text_layer, GColorClear);
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );
	text_layer_set_text( text_layer, "Data provided by Spitcast (www.spitcast.com)" );
	text_layer_set_font( text_layer, fonts_get_system_font( FONT_KEY_GOTHIC_14 ) );
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	text_layer_set_overflow_mode( text_layer, GTextOverflowModeWordWrap );
	
	window_set_click_config_provider( window, ( ClickConfigProvider ) splash_config_provider );
}

/********** LOCATIONS MENU **********/
static uint16_t menu_get_num_sections_callback( MenuLayer *menu_layer, void *data ) {
  return 2;
}

static uint16_t menu_get_num_rows_callback( MenuLayer *menu_layer, uint16_t section_index, void *data ) {
  switch ( section_index ) {
    case 0:
      return NUM_SPOTS;
			break;
    default:
      return 0;
			break;
  }
}

static int16_t menu_get_header_height_callback( MenuLayer *menu_layer, uint16_t section_index, void *data ) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback( GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data ) {
  switch ( section_index ) {
    case 0:
      menu_cell_basic_header_draw( ctx, cell_layer, "Orange County" );
      break;

    case 1:
      menu_cell_basic_header_draw( ctx, cell_layer, "Santa Cruz County" );
      break;
  }
}

static void menu_draw_row_callback( GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data ) {
  switch ( cell_index->section ) {
    case 0:
      switch ( cell_index->row ) {
        case 0:
          menu_cell_basic_draw( ctx, cell_layer, "Spot 1", NULL, NULL );
          break;

        case 1:
          menu_cell_basic_draw( ctx, cell_layer, "Spot 2", NULL, NULL );
          break;

        case 2:
          menu_cell_basic_draw( ctx, cell_layer, "Spot 3", NULL, NULL );
          break;
				
				case 3:
					menu_cell_basic_draw( ctx, cell_layer, "Spot 4", NULL, NULL );
          break;

				case 4:
				menu_cell_basic_draw( ctx, cell_layer, "Spot 5", NULL, NULL );
        break;
      }
      break;

    case 1:
      switch ( cell_index->row ) {
        case 0:
          menu_cell_title_draw( ctx, cell_layer, "Something Else" );
          break;
      }
  }
}

void menu_select_callback( MenuLayer *menu_layer, MenuIndex *cell_index, void *data ) {
  switch ( cell_index->row ) {
    case 0:
  			create_dashboard( "Spot 1" );
  			break;
  		case 1:
  			create_dashboard( "Spot 2" );
  			break;
  		case 2:
  			create_dashboard( "Spot 3" );
  			break;
  		case 3:
  			create_dashboard( "Spot 4" );
  			break;
  		case 4:
  			create_dashboard( "Spot 5" );
  			break;
  }
}

void window_load ( Window *window ) {	
	Layer *window_layer = window_get_root_layer( window );
	menu_layer = menu_layer_create( layer_get_frame( window_layer ) );
	
	menu_layer_set_callbacks( menu_layer, NULL, ( MenuLayerCallbacks ){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
		.draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

	menu_layer_set_click_config_onto_window( menu_layer, window );
	layer_add_child( window_layer, menu_layer_get_layer( menu_layer ) );
}

void window_unload( Window *window ) {
  menu_layer_destroy( menu_layer );

  for (int i = 0; i < NUM_RATINGS; i++) {
    gbitmap_deinit( &rating_icons[ i ] );
  }
}

void create_menu_screen( void ) {
	window = window_create();
	
	window_set_window_handlers( window, ( WindowHandlers ) {
    .load = window_load,
    .unload = window_unload,
  });
	
	window_stack_push( window, true );
}

/********** FORECAST **********/
void create_widget( char *text, int width_units, int x, int y, int rating ) {
	text_layer = text_layer_create( GRect( x,y,( width_units * SCREEN_WIDTH )/2,USABLE_HEIGHT/3 ) );
	text_layer_set_text( text_layer, text );
	
	text_layer_set_text_alignment( text_layer, GTextAlignmentCenter );
	layer_add_child( window_get_root_layer( window ), text_layer_get_layer( text_layer ) );
	
	rating_layer = text_layer_create( GRect( x,y+16,( width_units * SCREEN_WIDTH )/2,( USABLE_HEIGHT/3 )-16 ) );
	
	switch( rating ){
		case 1: text_layer_set_text( rating_layer, "Poor" ); break;
		case 2: text_layer_set_text( rating_layer, "Poor-Fair" ); break;
		case 3: text_layer_set_text( rating_layer, "Fair" ); break;
		case 4: text_layer_set_text( rating_layer, "Fair-Good" ); break;
		case 5: text_layer_set_text( rating_layer, "Good" ); break;
		case 0: text_layer_set_text( rating_layer, "!" ); break;
		default: text_layer_set_text( rating_layer, "=)" ); break;
	}
	
	text_layer_set_text_alignment( rating_layer, GTextAlignmentCenter );
	text_layer_set_background_color( rating_layer, GColorBlack );
	layer_add_child( text_layer_get_layer( text_layer ), text_layer_get_layer( rating_layer ) );	
}

void create_dashboard( Forecast *forecast ) {
	window = window_create();
	window_stack_push( window, true /* Animated */ );
	
	// create_widget( forecast.location,2,0,0,forecast.overall );
	// create_widget( "Swell",1,0,USABLE_HEIGHT/3,forecast.swell );
	// create_widget( "Tide",1,SCREEN_WIDTH/2,USABLE_HEIGHT/3,forecast.tide );
	// create_widget( "Wind",1,0,2*USABLE_HEIGHT/3,forecast.wind );
	// create_widget( "Warning",1,SCREEN_WIDTH/2,2*USABLE_HEIGHT/3,forecast.warning );
}

/********** MAIN APP LOOP **********/
void handle_init( void ) {
	gbitmap_init_with_resource( &splash_bitmap, RESOURCE_ID_SPLASH_ICON );
	
	create_splash_screen();
	// persist_read_data( 1, sizeof( forecast_array ), forecast_array );
}

void handle_deinit( void ) {
	window_destroy( window );
	// persist_write_data( 1, sizeof( forecast_array ), forecast_array );
}

int main( void ) {
	handle_init();
  app_event_loop();
	handle_deinit();
}