#ifndef __FORECAST_H
#define __FORECAST_H

typedef struct ForecastNode {
	int date;
	int hour;
	int conditions[ 4 ];
	char swell_size[ 2 ];		// Check to see if snprintf or int printing to watch works & change to int if appropriate
	struct ForecastNode *next_forecast;
} ForecastNode;

typedef struct TideForecastNode {
	int date;
	int hour;
	int tide_height;
	struct TideForecastNode *next_tide_forecast;
} TideForecastNode;

typedef struct {
	TideForecastNode *first_tide_forecast;
	char county_name[ 20 ];
} County;

typedef struct {
	ForecastNode *first_forecast;
	char name[ 20 ];
	County *county;
} Location;

enum condition_types {
	OVERALL = 0,				// This naming is a little inconsistent - should probably make up my mind between "general" and "overall"
	SWELL = 1,
	TIDE = 2,
	WIND = 3
};
	
ForecastNode *create_forecast( char *spot_name, char *county_name, int date, int hour, int general, int swell, int tide, int wind, char *swell_size );
TideForecastNode *create_tide_forecast( char *county_name, int date, int hour, int tide_height );

void expire_forecasts_before( int date, int hour );
void expire_tide_forecasts_before( int date, int hour );

int get_num_locations();

Location *get_location_by_index( int indexed_location );

char *get_county( Location *location );
char *get_spot_name( Location *location );

bool has_current_forecast( Location *location );
int get_current_conditions( Location *location, int condition_type );
char *get_current_swell_size( Location *location );
// int get_current_swell_size( Location *location );

void init_forecast_data();
void deinit_forecast_data();

#endif