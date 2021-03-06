// MEAT! This file handles all of the forecast data storage, insertion, and removal

#include <pebble.h>

#include <stdlib.h>
#include <time.h>

#include "forecast.h"
#include "utils.h"
#include "constants.h"
#include "display.h"

static Location locations[ NUM_SPOTS ];	// Array containing all locations
static County counties[ NUM_COUNTIES ];	// Array containing all counties

static int next_location = 0;	// Index for locations[]
static int next_county = 0;		// Index for counties[]

//========== Counties ==========
char *get_county_name( Location *location ){
	return location->county->county_name;
}

static County *get_county_by_name( char *county_name ){
	for( int i=0; i<NUM_COUNTIES; i++ ){
		if( ( strcmp( county_name, counties[ i ].county_name ) == 0 ) ){
			return( &counties[ i ] );
		}
	}
	return NULL;
}

static County *create_county( char *county_name ){
	counties[ next_county ].first_tide_forecast = NULL;
	strncpy( counties[ next_county ].county_name, county_name, sizeof( counties[ next_county ].county_name ) );
	
	APP_LOG( APP_LOG_LEVEL_DEBUG, "Creating county %s",counties[ next_county ].county_name );
	return( &counties[ next_county++ ] );
}

//========== Locations ==========
int get_num_locations(){
	return next_location;
}

Location *get_location_by_index( int indexed_location ){
	if( indexed_location < next_location ){
		return &locations[ indexed_location ];
	}
	return NULL;
}

static Location *get_location_by_spot( char *name ){
	for( int i=0; i<NUM_SPOTS; i++ ){
		if( ( strcmp( name, locations[ i ].name ) == 0 ) ){
			return( &locations[ i ] );
		}
	}
	return NULL;
}

char *get_spot_name( Location *location ){
	return location->name;
}

static Location *create_location( char *spot_name, char *county_name ){
	Location *location = &locations[ next_location ];
	County *county;
	
	location->first_forecast = NULL;
	strncpy( location->name, spot_name, sizeof( location->name ) );
	
	if( ( county = get_county_by_name( county_name ) ) == NULL ){
		county = create_county( county_name );
	}
	location->county = county;
	
	// Set location = locations[ next_location ] & increment next_location independently to facilitate menu scroll update
	next_location++;
	update_menu();
	
	APP_LOG( APP_LOG_LEVEL_DEBUG, "Creating location %s",locations[ next_location ].name );
	return( location );
}

//========== Forecast & TideForecast Creation & Expiration ==========
ForecastNode *create_forecast( char *spot_name, char *county_name, int date, int hour, int general, int swell, int tide, int wind, int swell_size ){
	// Check that forecast isn't already expired
	if( date > get_current_date() || ( date == get_current_date() && hour >= get_current_hour() ) ){
		Location *location;

		// Find location, or create a new location if not already existing
		if( ( location = get_location_by_spot( spot_name ) ) == NULL ){
			location = create_location( spot_name, county_name );
		}

		ForecastNode *current, *prev, *forecast;
		
		// Create the ForecastNode
		forecast = malloc( sizeof( ForecastNode ) );
		forecast->date = date;
		forecast->hour = hour;
		forecast->conditions[0] = general;
		forecast->conditions[1] = swell;
		forecast->conditions[2] = tide;
		forecast->conditions[3] = wind;
		forecast->swell_size = swell_size;

		// Insert the new ForecastNode
		if( ( current = location->first_forecast ) == NULL || ( current->date > forecast->date ) || ( current->date == forecast->date && current->hour > forecast->hour ) ){
			location->first_forecast = forecast;
			forecast->next_forecast = current;
		}
		else{ 
			prev = current; current = current->next_forecast;
			while( prev->next_forecast != NULL && ( current->date < forecast->date || ( current->date == forecast->date && current->hour < forecast->hour ) ) ){
				prev = current;
				current = current->next_forecast;
			}
			
			forecast->next_forecast = current;
			prev->next_forecast = forecast;
		}	
		return forecast;
	}
	else {
		APP_LOG( APP_LOG_LEVEL_DEBUG, "Skipping expired forecast from day %u hour %u", date, hour );
		return NULL;
	}
}

TideForecastNode *create_tide_forecast( char *county_name, int date, int hour, int tide_height ){
	// Check that tide forecast isn't already expired
	if( date > get_current_date() || ( date == get_current_date() && hour >= get_current_hour() ) ){
		County *county;

		if( ( county = get_county_by_name( county_name ) ) == NULL ){
			APP_LOG( APP_LOG_LEVEL_ERROR, "Missing county for %s", county_name );
		}

		TideForecastNode *current, *prev, *tide_forecast;
		
		// Create the TideForecastNode
		tide_forecast = malloc( sizeof( TideForecastNode ) );
		tide_forecast->date = date;
		tide_forecast->hour = hour;
		tide_forecast->tide_height = tide_height;

		// Insert the new TideForecastNode
		if( ( current = county->first_tide_forecast ) == NULL || ( current->date > tide_forecast->date ) || ( current->date == tide_forecast->date && current->hour > tide_forecast->hour ) ){
			county->first_tide_forecast = tide_forecast;
			tide_forecast->next_tide_forecast = current;
		}
		else{ 
			prev = current; current = current->next_tide_forecast;
			while( prev->next_tide_forecast != NULL && ( current->date < tide_forecast->date || ( current->date == tide_forecast->date && current->hour < tide_forecast->hour ) ) ){
				prev = current;
				current = current->next_tide_forecast;
			}
		
			tide_forecast->next_tide_forecast = current;
			prev->next_tide_forecast = tide_forecast;
		}	
		return tide_forecast;
	}
	else {
		APP_LOG( APP_LOG_LEVEL_DEBUG, "Skipping expired tide forecast from day %u hour %u", date, hour );
		return NULL;
	}
}

void expire_forecasts_before( int date, int hour ){
	for( int i=0; i<next_location; i++ ){
		ForecastNode *prev, *current;
		current = locations[ i ].first_forecast;
		
		while( current != NULL && ( current->date < date || ( current->date == date && current->hour < hour ) ) ){
			prev = current;
			current = current->next_forecast;
			free( prev );
		}
		locations[ i ].first_forecast = current;
	}
}

void expire_tide_forecasts_before( int date, int hour ){
	for( int i=0; i<next_county; i++ ){
		TideForecastNode *prev, *current;
		current = counties[ i ].first_tide_forecast;
		
		while( current != NULL && ( current->date < date || ( current->date == date && current->hour < hour ) ) ){
			prev = current;
			current = current->next_tide_forecast;
			free( prev );
		}
		counties[ i ].first_tide_forecast = current;
	}
}

//========== Forecast & TideForecast Accessors ==========
static ForecastNode *get_current_forecast( Location *location ){
	if( ( location->first_forecast != NULL ) && ( location->first_forecast->date == get_current_date() ) && ( location->first_forecast->hour == get_current_hour() ) ){
		return location->first_forecast;
	}
	else{
		return NULL;
	}
}

bool has_current_forecast( Location *location ){
	if ( get_current_forecast( location ) != NULL ){
		return true;
	}
	else{
		return false;
	}
}

int get_current_conditions( Location *location, int condition_type ){
	ForecastNode *forecast = get_current_forecast( location );
	return( forecast->conditions[ condition_type ] );
}

int get_current_swell_size( Location *location ){
	ForecastNode *forecast = get_current_forecast( location );
	return( forecast->swell_size );
}

//========== Data Management ==========
void init_forecast_data(){
// Check for persistent storage data
}

void deinit_forecast_data(){
	for( int i=0; i<NUM_SPOTS; i++ ){
		ForecastNode *prev, *current;
		current = locations[ i ].first_forecast;
		
		while( current != NULL ){
			prev = current;
			current = current->next_forecast;
			free( prev );
		}	
	}
	for( int i=0; i<NUM_COUNTIES; i++ ){
		TideForecastNode *prev, *current;
		current = counties[ i ].first_tide_forecast;
		
		while( current != NULL ){
			prev = current;
			current = current->next_tide_forecast;
			free( prev );
		}	
	}
}
