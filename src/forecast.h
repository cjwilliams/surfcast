#ifndef __FORECAST_H
#define __FORECAST_H

typedef struct {
	int date;
	int hour;
	int conditions[4];
	char swell_size;
	char *spot_name;
} Forecast;

typedef struct {
	int date;
	int hour;
	char tide_height;
	char *county_name;
} TideForecast;

typedef struct {
	int current_index;
	char *county;
	char *name;
} Location;

enum condition_types {
	OVERALL = 0,				// This naming is a little inconsistent - should probably make up my mind between "general" and "overall"
	SWELL = 1,
	TIDE = 2,
	WIND = 3
};
	
Forecast *create_forecast( char *spot_name, char *county_name, int date, int hour, int general, int swell, int tide, int wind, char swell_size );
TideForecast *create_tide_forecast( char *county_name, int date, int hour, float tide_height );

Location *get_location( int indexed_location );
char *get_county( Location *location );
char *get_spot_name( Location *location );
int get_current_conditions( Location *location, int condition_type );
char get_current_swell_size( Location *location );
float *get_tide_heights( Location *location );

#endif