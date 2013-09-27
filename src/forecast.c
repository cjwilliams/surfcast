// Make sure "forecast" and "location" are not globals

#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include <stdlib.h>
#include <time.h>

#include "forecast.h"
#include "utils.h"
#include "constants.h"

static Forecast forecasts[ NUM_TOTAL_FORECASTS ];
static TideForecast tide_forecasts[ NUM_TOTAL_FORECASTS ];
static Location locations[ NUM_SPOTS ];
float tide_heights[ 10 ];

static int next_forecast = 0;
static int next_tide_forecast = 0;
static int next_location = 0;

Location *create_location( char *spot_name, char *county ){
	locations[ next_location ].current_index = -1;
	locations[ next_location ].current_tide_index = -1;
	strncpy( locations[ next_location ].name, spot_name, sizeof( locations[ next_location ].name ) );
	strncpy( locations[ next_location ].county, county, sizeof( locations[ next_location ].county ) );
	
	APP_LOG( APP_LOG_LEVEL_DEBUG, "Creating location %s",locations[ next_location ].name );
	return( &locations[ next_location++ ] );
}

Forecast *create_forecast( char *spot_name, char *county_name, int date, int hour, int general, int swell, int tide, int wind, char *swell_size ){
	if( next_forecast < NUM_TOTAL_FORECASTS ){
		Location *location;

		forecasts[ next_forecast ].date = date;
		forecasts[ next_forecast ].hour = hour;
		forecasts[ next_forecast ].conditions[0] = general;
		forecasts[ next_forecast ].conditions[1] = swell;
		forecasts[ next_forecast ].conditions[2] = tide;
		forecasts[ next_forecast ].conditions[3] = wind;
		strncpy( forecasts[ next_forecast ].swell_size, swell_size, sizeof( forecasts[ next_forecast ].swell_size ) );
	
		if( ( location = get_location_by_spot( spot_name ) ) == NULL ){
			location = create_location( spot_name, county_name );
		}
		forecasts[ next_forecast ].location = location;
	}
	else{
		APP_LOG(APP_LOG_LEVEL_ERROR, "Almost had a Buffer Overflow");
	}
	
	return( &forecasts[ next_forecast++ ] );
}

TideForecast *create_tide_forecast( char *county_name, int date, int hour, int tide_height ){
	if( next_forecast < NUM_TOTAL_FORECASTS ){
		tide_forecasts[ next_tide_forecast ].date = date;
		tide_forecasts[ next_tide_forecast ].hour = hour;
		tide_forecasts[ next_tide_forecast ].tide_height = tide_height;	
		tide_forecasts[ next_tide_forecast ].location = get_location_by_county( county_name );	
	}
	else{
		APP_LOG(APP_LOG_LEVEL_ERROR, "Almost had a Buffer Overflow");
	}
	
	return( &tide_forecasts[ next_tide_forecast++ ] ); 
}

void update_current_indices( void ){
	int date = get_current_date();
	int hour = get_current_hour();
	
	for( int i=0; i<NUM_TOTAL_FORECASTS; i++ ){
		if( ( forecasts[ i ].date == date ) && ( forecasts[ i ].hour == hour ) ){
			forecasts[ i ].location->current_index = i;
		}
	}
	for( int j=0; j<NUM_TOTAL_FORECASTS; j++ ){
		if( ( tide_forecasts[ j ].date == date ) && ( tide_forecasts[ j ].hour == hour ) ){
			tide_forecasts[ j ].location->current_tide_index = j;
		}
	}
}

Location *get_location_by_index( int indexed_location ){
	return &locations[ indexed_location ];
}

Location *get_location_by_spot( char *name ){
	for( int i=0; i<NUM_SPOTS; i++ ){
		if( ( strcmp( name, locations[ i ].name ) == 0 ) ){
			return( &locations[ i ] );
		}
	}
	return NULL;
}

Location *get_location_by_county( char *county ){
	for( int i=0; i<NUM_SPOTS; i++ ){
		if( ( strcmp( county, locations[ i ].county ) == 0 ) ){
			return( &locations[ i ] );
		}
	}
	return NULL;
}

char *get_county( Location *location ){
	return location->county;
}

char *get_spot_name( Location *location ){
	return location->name;
}

static Forecast *get_current_forecast( Location *location ){
	return &forecasts[ location->current_index ];
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
// int get_tide_heights( Location *location ){
// 	Forecast *forecast = get_current_forecast( location );
// 	
// 	/* More stuff here */
// 	return &tide_heights[ 0 ];
// }
