#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include "app_msg.h"
#include "constants.h"
#include "forecast.h"
#include "utils.h"

static int done = 0;

enum {
	REQUEST_STATUS_KEY = 0xF,
	
	SPOT_KEY = 0x0,
	COUNTY_KEY = 0x1,
	
	DATE_KEY = 0x2,
  HOUR_KEY = 0x3,
	
	GENERAL_KEY = 0x4,
	SWELL_KEY = 0x5,
	TIDE_KEY = 0x6,
	WIND_KEY = 0x7,
	SWELL_SIZE_KEY = 0x8,
	
	TIDE_HEIGHT_KEY = 0x9
};

static void in_received_handler( DictionaryIterator *received, void *context ) {
	APP_LOG( APP_LOG_LEVEL_DEBUG_VERBOSE, "Inbound Message Received Handler" );
	
	Tuple *tuple;
	
	tuple = dict_find( received, REQUEST_STATUS_KEY );
	if( (int)tuple->value->uint32 == STOP_FLAG ){ 
		done = 1; 
		return;
	}
	
	tuple = dict_find( received, SPOT_KEY );  
	if( tuple ) {
		create_forecast( 
			tuple->value->cstring,
			dict_find( received, COUNTY_KEY )->value->cstring, 
			dict_find( received, DATE_KEY )->value->uint32,
			dict_find( received, HOUR_KEY )->value->uint32,
			dict_find( received, GENERAL_KEY )->value->uint32,
			dict_find( received, SWELL_KEY )->value->uint32,
			dict_find( received, TIDE_KEY )->value->uint32,
			dict_find( received, WIND_KEY )->value->uint32,
			dict_find( received, SWELL_SIZE_KEY )->value->cstring
		);
		APP_LOG( APP_LOG_LEVEL_DEBUG, "SPOT: %s, DATE: %lu, HOUR: %lu, GENERAL: %lu, SWELL: %lu, TIDE: %lu, WIND: %lu, SWELL_SIZE: %s", 
			tuple->value->cstring,
			// dict_find( received, COUNTY_KEY )->value->cstring, 
			dict_find( received, DATE_KEY )->value->uint32,
			dict_find( received, HOUR_KEY )->value->uint32,
			dict_find( received, GENERAL_KEY )->value->uint32,
			dict_find( received, SWELL_KEY )->value->uint32,
			dict_find( received, TIDE_KEY )->value->uint32,
			dict_find( received, WIND_KEY )->value->uint32,
			dict_find( received, SWELL_SIZE_KEY )->value->cstring
		);
		return;
	}
	
	tuple = dict_find( received, TIDE_HEIGHT_KEY );  
	if( tuple ) {
		create_tide_forecast(
			dict_find( received, COUNTY_KEY )->value->cstring, 
			dict_find( received, DATE_KEY )->value->uint32,
			dict_find( received, HOUR_KEY )->value->uint32,
			tuple->value->uint32
		);
		APP_LOG( APP_LOG_LEVEL_DEBUG, "COUNTY: %s, DATE: %lu, HOUR: %lu, TIDE_HEIGHT: %lu", 
			dict_find( received, COUNTY_KEY )->value->cstring, 
			dict_find( received, DATE_KEY )->value->uint32,
			dict_find( received, HOUR_KEY )->value->uint32,
			tuple->value->uint32
		);
		return;
	}
	APP_LOG( APP_LOG_LEVEL_WARNING, "Expecting Forecast or TideForecast, Got Something Else" );
}

static void in_dropped_handler( void *context, AppMessageResult reason ) {	
	APP_LOG( APP_LOG_LEVEL_WARNING, "Inbound Message Dropped Handler" );
	debug_reason( reason );
	// Does this do anything? Seems to auto retry after being called...?
}

static void out_sent_handler( DictionaryIterator *sent, void *context ) {
	APP_LOG( APP_LOG_LEVEL_DEBUG_VERBOSE, "Outbound Message Sent Handler" );
	// Does this do anything?
}

static void out_failed_handler( DictionaryIterator *failed, AppMessageResult reason, void *context ) {
	APP_LOG( APP_LOG_LEVEL_WARNING, "Outbound Message Failed Handler" );
	debug_reason( reason );
	// Does this do anything? Seems to auto retry after being called...?
}

static void out_next_handler( AppMessageResult result, void *context ) {
	APP_LOG( APP_LOG_LEVEL_DEBUG, "Outbound Message Next Handler" );
	
	if( done ){ return; } 
	else if( result != APP_MSG_OK ){ 
		get_next_forecast( RETRY_MESSAGE ); 
		debug_reason( result );
	} 
	else { get_next_forecast( NEW_MESSAGE ); }
}

static AppMessageCallbacksNode app_msg_callbacks = {
  .callbacks = {
		.out_sent = out_sent_handler,							
		.out_failed = out_failed_handler,
		.in_received = in_received_handler,	
		.in_dropped = in_dropped_handler,
		.out_next = out_next_handler, /* This is not documented/supported? */
  }
};

void app_message_init( void ) {
	app_comm_set_sniff_interval( SNIFF_INTERVAL_REDUCED );
	app_message_open( MAX_BUFFER_SIZE, MAX_BUFFER_SIZE );
	
	if( app_message_register_callbacks( &app_msg_callbacks ) != APP_MSG_OK ){
		return;
	}
	
	init_forecast_data();
		
	get_next_forecast( NEW_MESSAGE );
	start_timer();
}

void app_message_deinit( void ) {
	stop_timer();
  app_message_out_release();
	app_message_deregister_callbacks( &app_msg_callbacks );
	deinit_forecast_data();
}

void get_next_forecast( uint8_t status_flag ) {
	AppMessageResult reason;
	DictionaryResult result;
	DictionaryIterator *iter;
	
	if ( ( reason = app_message_out_get( &iter ) ) != APP_MSG_OK ) {
		debug_reason( reason );
		set_stopped_flag();
		return;
	}

	if ( ( result = dict_write_uint8( iter, REQUEST_STATUS_KEY, status_flag ) ) != DICT_OK ) {
		debug_dictionary_result( result );
		set_stopped_flag();
		return;
	}

	dict_write_end( iter );
  app_message_out_send();
  app_message_out_release();
}