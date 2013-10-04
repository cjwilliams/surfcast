// ============================================================ SURFCAST ============================================================
// Description: A Pebble watch application to track & display surfing condition forecasts (provided by www.spitcast.com)
//
// Make sure the following component files are present before compiling and installing:
//
//		app_msg.c/app_msg.h 		- Handles all communications with PebbleKitJS
//		constants.h							- All non-static macros/constants are defined here
//		display.c/display.h			- Handles all rendering of data/images for Surfcast
//		forecast.c/forecast.h		- Handles management of forecast & tide forecast data
//		utils.c/utils.h					- Provides miscellaneous utility methods for use in other parts of Surfcast
//
//		pebble-js-app.js				- Queries the Spitcast API & manages the PebbleKitJS side of forecast communications
//
// Some images are provided for use with this application (surfboard.png, poor.png, pf.png, fair.png, fg.png, good.png)
// ==================================================================================================================================

#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include "app_msg.h"
#include "display.h"
#include "utils.h"

static void init( void ) {
	tick_timer_service_subscribe( HOUR_UNIT, set_current_datetime );
	init_windows();
	app_message_init();
}

static void deinit( void ) {
	app_message_deinit();
	deinit_windows();
	tick_timer_service_unsubscribe();
}

int main( void ) {
  init();
	app_event_loop();
  deinit();
}
