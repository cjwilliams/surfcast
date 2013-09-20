#ifndef __DEFS_H
#define __DEFS_H

#define NUM_LOCATIONS 5		/* This is bad, need to keep track of locations array size better */
#define NUM_FORECASTS 24
#define MAX_BUFFER_SIZE 256

// typedef struct {
// 	// struct tm time;
// 	int hour;
// 	int overall;
// 	int swell;
// 	int tide;
// 	int wind;
// 	int swell_size;
// 	int tide_height;	/* Not used yet */
// 	bool isset;
// } Forecast;
// 
typedef struct {
	int spot_id;
	char *spot_name;
	// Forecast forecasts[ NUM_FORECASTS ];
} Location;

enum {
	REQUEST_KEY = 0x0,						// TUPLE_INT
	DATA_KEY = 0x1,						// TUPLE_BYTE_ARRAY
	HOUR_KEY = 0x2,								// TUPLE_INT
  OVERALL_KEY = 0x3,  					// TUPLE_INT
	SWELL_KEY = 0x4,  						// TUPLE_INT
	TIDE_KEY = 0x5,  							// TUPLE_INT
	WIND_KEY = 0x6,  							// TUPLE_INT
	SWELL_SIZE_KEY = 0x7,  				// TUPLE_INT
	TIDE_HEIGHT_KEY = 0x8,  			// TUPLE_INT
	LOCATION_KEY = 0x9,  					// TUPLE_INT
	DURATION_KEY = 0xA
};

static uint32_t CONDITION_ICONS[] = {
  RESOURCE_ID_POOR_COND,
  RESOURCE_ID_PF_COND,
  RESOURCE_ID_FAIR_COND,
  RESOURCE_ID_FG_COND,
	RESOURCE_ID_GOOD_COND
};

#endif