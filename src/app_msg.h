#ifndef __APP_MSG_H
#define __APP_MSG_H

void app_message_init( void );
void app_message_deinit( void );

// get_next_forecast( 1 ) - normal
// get_next_forecast( 2 ) - missed message
// status_flag = 0 - done transmitting
void get_next_forecast( uint8_t status_flag );

#endif