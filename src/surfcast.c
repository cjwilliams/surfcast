// TODO:
// Review data structures for appropriateness
// Persist write
// Draw tide heights

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
