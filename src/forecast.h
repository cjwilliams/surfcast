#ifndef __FORECAST_H
#define __FORECAST_H

// Structure to store all standard forecasts in a linked list format
typedef struct ForecastNode {
	int date;
	int hour;
	int conditions[ 4 ];
	int swell_size;
	struct ForecastNode *next_forecast;
} ForecastNode;

// Structure to store all tide forecasts in a linked list format
typedef struct TideForecastNode {
	int date;
	int hour;
	int tide_height;
	struct TideForecastNode *next_tide_forecast;
} TideForecastNode;

// Structure to store a county & its first available tide forecast
typedef struct {
	TideForecastNode *first_tide_forecast;
	char county_name[ 20 ];
} County;

// Structure to store a location/spot, its associate county, and its first available standard forecast
typedef struct {
	ForecastNode *first_forecast;
	char name[ 20 ];
	County *county;
} Location;

// All of the different conditions provided in a standard forecast
enum condition_types {
	OVERALL = 0,				// This naming is a little inconsistent - should probably make up my mind between "general" and "overall"
	SWELL = 1,
	TIDE = 2,
	WIND = 3
};
	
ForecastNode *create_forecast( char *spot_name, char *county_name, int date, int hour, int general, int swell, int tide, int wind, int swell_size );
TideForecastNode *create_tide_forecast( char *county_name, int date, int hour, int tide_height );

void expire_forecasts_before( int date, int hour );
void expire_tide_forecasts_before( int date, int hour );

int get_num_locations();

Location *get_location_by_index( int indexed_location );	// Returns location if found, NULL otherwise

char *get_county_name( Location *location );	// Returns county string, location cannot be NULL
char *get_spot_name( Location *location );	// Returns spot name, location cannot be NULL

bool has_current_forecast( Location *location );
int get_current_conditions( Location *location, int condition_type );
int get_current_swell_size( Location *location );

void init_forecast_data();
void deinit_forecast_data();

#endif