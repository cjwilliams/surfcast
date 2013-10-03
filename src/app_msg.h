#ifndef __APP_MSG_H
#define __APP_MSG_H

void app_message_init( void );
void app_message_deinit( void );

// get_next_forecast( STOP_FLAG ) - not used ( '0' is used by PebbleKitJS to signal end of transmissions )
// get_next_forecast( NEW_MESSAGE ) - transmit next forecast
// get_next_forecast( RETRY_MESSAGE ) - retry dropped/failed transmission
// get_next_forecast( FETCH_ADDITIONAL ) - fetch NUM_FORECASTS, starting with the current day
void get_next_forecast( uint8_t status_flag );

enum REQUEST_STATUSES {
	STOP_FLAG = 0x0,
	NEW_MESSAGE = 0x1,
	RETRY_MESSAGE = 0x2,
	FETCH_ADDITIONAL = 0x3,
	READY_FLAG = 0x4
};

#endif