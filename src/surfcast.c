// TODO:
// Review data structures for appropriateness
// Persist write
// Draw tide heights

#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include "app_msg.h"
#include "display.h"

static void init( void ) {
	init_windows();
	app_message_init();
}

static void deinit( void ) {
	app_message_deinit();
	deinit_windows();
}

int main( void ) {
  init();
	app_event_loop();
  deinit();
}
