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

Forecast *create_forecast( char *spot_name, char *county_name, int date, int hour, int general, int swell, int tide, int wind, char swell_size );
TideForecast *create_tide_forecast( char *county_name, int date, int hour, float tide_height );

Forecast *get_current_forecast_at_location( int indexed_location );
int get_conditions( Forecast *forecast, int category );
char *get_location( Forecast *forecast );
char *get_county( Forecast *forecast );
char get_current_swell_size( Forecast *forecast );

#endif