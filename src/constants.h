#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#define MAX_BUFFER_SIZE 124		// Used for AppMessage
#define WATCHDOG_DELAY 20000	// 20000ms = 20s

#define NUM_SPOTS 5				// This is a little sloppy, should make this a calculated value based on JS-provided results
#define NUM_COUNTIES 5		// This is a little sloppy, should make this a calculated value based on JS-provided results

#define NUM_CONDITIONS 5	// Number of total conditions (Poor, Poor-Fair, Fair, Fair-Good, Good)

// Values obtained from Pebble Forums - should replace these with bounds from layer_get_frame( Layer *layer )
#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168
#define STATUS_BAR_HEIGHT 16
#define USABLE_HEIGHT (( SCREEN_HEIGHT )-( STATUS_BAR_HEIGHT ))

#define MAX_LAYERS 5	// Maximum number of forecast_title_layers, forecast_icon_layers or forecast_value_layers

#endif