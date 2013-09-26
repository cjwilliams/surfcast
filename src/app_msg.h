#ifndef __APP_MSG_H
#define __APP_MSG_H

void app_message_init( void );
void app_message_deinit( void );

// get_next_forecast( 0 ) - not used ( '0' is used by PebbleKitJS to signal end of transmissions )
// get_next_forecast( 1 ) - transmit next forecast
// get_next_forecast( 2 ) - retry dropped/failed transmission
// get_next_forecast( 3 ) - fetch NUM_FORECASTS, starting with the current day
void get_next_forecast( uint8_t status_flag );

#endif