#include <pebble.h>

#include "app_msg.h"
#include "constants.h"
#include "forecast.h"
#include "utils.h"

// Key values for AppMessage Dictionary
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

static int request_status = 0;

void send_message( void ){
	AppMessageResult reason;
	DictionaryResult result;
	DictionaryIterator *iter;
	
	if ( ( reason = app_message_outbox_begin( &iter ) ) != APP_MSG_OK ) {
		APP_LOG( APP_LOG_LEVEL_WARNING, "Outbox Begin Failed" );
		debug_reason( reason );	// utils.h
		return;
	}

	if ( ( result = dict_write_uint8( iter, REQUEST_STATUS_KEY, request_status ) ) != DICT_OK ) {
		APP_LOG( APP_LOG_LEVEL_WARNING, "Dictionary Write Failed" );
		debug_dictionary_result( result );	// utils.h
		return;
	}

	dict_write_end( iter );
  app_message_outbox_send();
}

// Called when a message is received from PebbleKitJS
static void in_received_handler( DictionaryIterator *received, void *context ) {
	APP_LOG( APP_LOG_LEVEL_DEBUG_VERBOSE, "Inbound Message Received Handler" );
	
	Tuple *tuple;
	
	tuple = dict_find( received, SPOT_KEY );	// Checks for SPOT_KEY, which only exists in AppMessages with a standard forecast
	if( tuple != NULL ) {
		create_forecast( 
			tuple->value->cstring,
			dict_find( received, COUNTY_KEY )->value->cstring, 
			dict_find( received, DATE_KEY )->value->uint32,
			dict_find( received, HOUR_KEY )->value->uint32,
			dict_find( received, GENERAL_KEY )->value->uint32,
			dict_find( received, SWELL_KEY )->value->uint32,
			dict_find( received, TIDE_KEY )->value->uint32,
			dict_find( received, WIND_KEY )->value->uint32,
			dict_find( received, SWELL_SIZE_KEY )->value->uint32
		);
		APP_LOG( APP_LOG_LEVEL_DEBUG, "SPOT: %s, DATE: %lu, HOUR: %lu, GENERAL: %lu, SWELL: %lu, TIDE: %lu, WIND: %lu, SWELL_SIZE: %s", 
			tuple->value->cstring,
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
	
	tuple = dict_find( received, TIDE_HEIGHT_KEY );	// Checks for TIDE_HEIGHT_KEY, which only exists in AppMessages with a tide forecast
	if( tuple != NULL ) {
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
	// Default error when AppMessage is not a standard forecast or a tide forecast
	APP_LOG( APP_LOG_LEVEL_WARNING, "Expecting Forecast or TideForecast, Got Something Else" );	
}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler( AppMessageResult reason, void *context ) {	
	APP_LOG( APP_LOG_LEVEL_WARNING, "Inbound Message Dropped Handler" );
	debug_reason( reason );
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler( DictionaryIterator *failed, AppMessageResult reason, void *context ) {
	APP_LOG( APP_LOG_LEVEL_WARNING, "Outbound Message Failed Handler" );
	debug_reason( reason );
	send_message();
}

void app_message_init( void ) {
	app_message_register_inbox_received( in_received_handler ); 
	app_message_register_inbox_dropped( in_dropped_handler ); 
	app_message_register_outbox_failed( out_failed_handler );
		
	app_comm_set_sniff_interval( SNIFF_INTERVAL_REDUCED );
	app_message_open( app_message_inbox_size_maximum(), app_message_outbox_size_maximum() );
	
	request_status = NEW_MESSAGE;
	send_message();
}

void app_message_deinit( void ) {
	app_message_deregister_callbacks();
}