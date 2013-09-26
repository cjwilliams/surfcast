// TODO:
// Review data structures for appropriateness
// Persist write
// Draw tide heights

#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include "app_msg.h"
#include "display.h"

#define MY_UUID { 0x55, 0xFF, 0x55, 0x62, 0x68, 0xBF, 0x4B, 0xF8, 0x83, 0xD4, 0x44, 0xE6, 0xB0, 0x9E, 0x0D, 0x59 }
PBL_APP_INFO( MY_UUID,
             "Pebble Surfcast", "Girlgrammer",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP );

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
