#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include "utils.h"
#include "constants.h"

static AppTimer *timer;
static int stopped = 0;

extern Window *window;
extern TextLayer *text_layer;

extern void get_next_forecast( uint8_t flag );

void set_stopped_flag( void) {
	stopped = 1;
}

void reset_stopped_flag( void ) {
	stopped = 0;
}

static void watchdog_timer_check( void *context ) {
	if( stopped == 1 ) {
		reset_stopped_flag();
		get_next_forecast( 1 );
		test_print("Stopped");
	}
	start_timer();
}

void start_timer( void ) {
	timer = app_timer_register( WATCHDOG_DELAY, watchdog_timer_check, NULL);
}

void stop_timer( void ) {
	test_print("Cancel Timer");
	app_timer_cancel( timer );
}

// ========== Debugging ==========
void test_print( char *str ) {
	text_layer_set_text( text_layer, str );
	psleep( 500 );
}

void debug_reason( AppMessageResult reason ) {
	switch( reason ) {
		case APP_MSG_OK: test_print( "OK" ); break;
		case APP_MSG_SEND_TIMEOUT: test_print( "Timeout" ); break;	
		case APP_MSG_SEND_REJECTED: test_print( "Rejected" ); break;
		case APP_MSG_NOT_CONNECTED: test_print( "Not Connected" ); break; 	
		case APP_MSG_APP_NOT_RUNNING: test_print( "Not Running" ); break; 	
		case APP_MSG_INVALID_ARGS: test_print( "Invalid Args" ); break; 	
		case APP_MSG_BUSY: test_print( "Busy" ); break; 	
		case APP_MSG_BUFFER_OVERFLOW: test_print( "Overflow" ); break;
		case APP_MSG_ALREADY_RELEASED: test_print( "Released" ); break;
		case APP_MSG_CALLBACK_ALREADY_REGISTERED: test_print( "Registered" ); break;
		case APP_MSG_CALLBACK_NOT_REGISTERED: test_print( "Not Registered" ); break;
	}
}

void debug_dictionary_result( DictionaryResult result ) {
	switch( result ) {
		case DICT_OK: test_print( "OK" ); break;
		case DICT_NOT_ENOUGH_STORAGE: test_print( "No Storage" ); break;
		case DICT_INVALID_ARGS: test_print( "Invalid Args" ); break; 	
		case DICT_INTERNAL_INCONSISTENCY: test_print( "Internal Inconsistency" ); break;
	}
}