#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#define MAX_BUFFER_SIZE 124
#define WATCHDOG_DELAY 20000

#define NUM_SPOTS 5				// This is a little sloppy, should make this a calculated value based on JS-provided results
#define NUM_COUNTIES 5		// This is a little sloppy, should make this a calculated value based on JS-provided results

#define NUM_CONDITIONS 5

#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168
#define STATUS_BAR_HEIGHT 16
#define USABLE_HEIGHT (( SCREEN_HEIGHT )-( STATUS_BAR_HEIGHT ))

#define MAX_LAYERS 5

#endif