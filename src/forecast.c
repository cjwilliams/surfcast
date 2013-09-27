// Make sure "forecast" and "location" are not globals

#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include "forecast.h"
#include "utils.h"
#include "constants.h"

static Forecast forecasts[ NUM_FORECASTS ];
static TideForecast tide_forecasts[ NUM_FORECASTS ];
static Location locations[ NUM_SPOTS ];
float tide_heights[ 10 ];

static int next_forecast = 0;
static int next_tide_forecast = 0;
static int next_location = 0;

Forecast *create_forecast( char *spot_name, char *county_name, int date, int hour, int general, int swell, int tide, int wind, char *swell_size ){
	forecasts[ next_forecast ].spot_name = spot_name;
	forecasts[ next_forecast ].date = date;
	forecasts[ next_forecast ].hour = hour;
	forecasts[ next_forecast ].conditions[0] = general;
	forecasts[ next_forecast ].conditions[1] = swell;
	forecasts[ next_forecast ].conditions[2] = tide;
	forecasts[ next_forecast ].conditions[3] = wind;
	forecasts[ next_forecast ].swell_size = swell_size;
	
	// if( /* locations does not contain spot_name */ ){
	// 	/* add spot_name to locations */
	// 	/* add county_name to locations */
	// 	next_location++;
	// }

	return( &forecasts[ next_forecast++ ] );
}

TideForecast *create_tide_forecast( char *county_name, int date, int hour, float tide_height ){
	tide_forecasts[ next_tide_forecast ].county_name = county_name;
	tide_forecasts[ next_tide_forecast ].date = date;
	tide_forecasts[ next_tide_forecast ].hour = hour;
	tide_forecasts[ next_tide_forecast ].tide_height = tide_height;
	
	return( &tide_forecasts[ next_tide_forecast++ ] ); 
}

static Forecast *get_current_forecast( Location *location ){
	return &forecasts[ location->current_index ];
}

Location *get_location( int indexed_location ){
	return &locations[ indexed_location ];
}

char *get_county( Location *location ){
	return location->county;
}

char *get_spot_name( Location *location ){
	return location->name;
}

int get_current_conditions( Location *location, int condition_type ){
	Forecast *forecast = get_current_forecast( location );
	return( forecast->conditions[ condition_type ] );
}

char *get_current_swell_size( Location *location ){
	Forecast *forecast = get_current_forecast( location );
	return( forecast->swell_size );
}

// This could probably be memory-optimized by using ints, since all we're doing is drawing the relationships between points
float *get_tide_heights( Location *location ){
	Forecast *forecast = get_current_forecast( location );
	
	/* More stuff here */
	return &tide_heights[ 0 ];
}
