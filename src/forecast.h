#ifndef __FORECAST_H
#define __FORECAST_H

typedef struct {
	int current_index;
	int current_tide_index;
	char county[ 20 ];
	char name[ 20 ];
} Location;

typedef struct {
	int date;
	int hour;
	int conditions[ 4 ];
	char swell_size[ 2 ];
	Location *location;
} Forecast;

typedef struct {
	int date;
	int hour;
	int tide_height;
	Location *location;
} TideForecast;

enum condition_types {
	OVERALL = 0,				// This naming is a little inconsistent - should probably make up my mind between "general" and "overall"
	SWELL = 1,
	TIDE = 2,
	WIND = 3
};
	
Forecast *create_forecast( char *spot_name, char *county_name, int date, int hour, int general, int swell, int tide, int wind, char *swell_size );
TideForecast *create_tide_forecast( char *county_name, int date, int hour, int tide_height );

void update_current_indices( void );

Location *get_location_by_index( int indexed_location );
Location *get_location_by_spot( char *name );
Location *get_location_by_county( char *county );

char *get_county( Location *location );
char *get_spot_name( Location *location );

int get_current_conditions( Location *location, int condition_type );
char *get_current_swell_size( Location *location );
int get_tide_heights( Location *location );

#endif