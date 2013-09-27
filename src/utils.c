#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include "utils.h"
#include "constants.h"
#include "app_msg.h"
#include "forecast.h"

static AppTimer *timer;
static int stopped = 0;
static int date;
static int hour;

// ========== Timekeeping ==========
bool set_current_datetime( void ){
	time_t now = time( NULL );
  struct tm *current_time = localtime( &now );
	
	if( date != current_time->tm_mday || hour != current_time->tm_hour ){
		date = current_time->tm_mday;
		hour = current_time->tm_hour;
		
		APP_LOG( APP_LOG_LEVEL_INFO, "Setting current date/time to %u(date) %u(time)", date, hour );
		
		update_current_indices();

		return true;
	}
	
	return false;
}

void set_stopped_flag( void) {
	stopped = 1;
}

void reset_stopped_flag( void ) {
	stopped = 0;
}

static void watchdog_timer_check( void *context ) {
	if( 1 == stopped ) {
		APP_LOG( APP_LOG_LEVEL_INFO, "Transmissions are Stopped - Restarting..." );
		reset_stopped_flag();
		get_next_forecast( 1 );
	}
	
	set_current_datetime();
	start_timer();
}

void start_timer( void ) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Watchdog Timer Started");
	timer = app_timer_register( WATCHDOG_DELAY, watchdog_timer_check, NULL);
}

void stop_timer( void ) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Watchdog Timer Stopped");
	app_timer_cancel( timer );
}

int get_current_date( void ){
	return date;
}

int get_current_hour( void ){
	return hour;
}

// ========== Debugging ==========
void debug_reason( AppMessageResult reason ) {
	switch( reason ) {
		case APP_MSG_OK: APP_LOG( APP_LOG_LEVEL_DEBUG_VERBOSE, "AppMessage: OK" ); break;
		case APP_MSG_SEND_TIMEOUT: APP_LOG( APP_LOG_LEVEL_DEBUG, "AppMessage: Send Timeout" ); break;	
		case APP_MSG_SEND_REJECTED: APP_LOG( APP_LOG_LEVEL_DEBUG, "AppMessage: Send Rejected" ); break;
		case APP_MSG_NOT_CONNECTED: APP_LOG( APP_LOG_LEVEL_DEBUG, "AppMessage: Not Connected" ); break; 	
		case APP_MSG_APP_NOT_RUNNING: APP_LOG( APP_LOG_LEVEL_DEBUG, "AppMessage: App Not Running" ); break; 	
		case APP_MSG_INVALID_ARGS: APP_LOG( APP_LOG_LEVEL_DEBUG, "AppMessage: Invalid Args" ); break; 	
		case APP_MSG_BUSY: APP_LOG( APP_LOG_LEVEL_DEBUG, "AppMessage: Busy" ); break; 	
		case APP_MSG_BUFFER_OVERFLOW: APP_LOG( APP_LOG_LEVEL_DEBUG, "AppMessage: Buffer Overflow" ); break;
		case APP_MSG_ALREADY_RELEASED: APP_LOG( APP_LOG_LEVEL_DEBUG, "AppMessage: Already Released" ); break;
		case APP_MSG_CALLBACK_ALREADY_REGISTERED: APP_LOG( APP_LOG_LEVEL_DEBUG, "AppMessage: Callback Already Registered" ); break;
		case APP_MSG_CALLBACK_NOT_REGISTERED: APP_LOG( APP_LOG_LEVEL_DEBUG, "AppMessage: Callback Not Registered" ); break;
	}
}

void debug_dictionary_result( DictionaryResult result ) {
	switch( result ) {
		case DICT_OK: APP_LOG( APP_LOG_LEVEL_DEBUG_VERBOSE, "Dictionary: OK" ); break;
		case DICT_NOT_ENOUGH_STORAGE: APP_LOG( APP_LOG_LEVEL_DEBUG, "Dictionary: Not Enough Storage" ); break;
		case DICT_INVALID_ARGS: APP_LOG( APP_LOG_LEVEL_DEBUG, "Dictionary: Invalid Args" ); break; 	
		case DICT_INTERNAL_INCONSISTENCY: APP_LOG( APP_LOG_LEVEL_DEBUG, "Dictionary: Internal Inconsistency" ); break;
	}
}