#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#define MAX_BUFFER_SIZE 124
#define WATCHDOG_DELAY 20000

#define NUM_FORECASTS 24	// This is a little sloppy, should make this a calculated value based on JS-provided results
#define NUM_SPOTS 5				// This is a little sloppy, should make this a calculated value based on JS-provided results
#define NUM_TOTAL_FORECASTS NUM_FORECASTS*NUM_SPOTS

#define NUM_CONDITIONS 5

#endif