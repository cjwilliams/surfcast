#ifndef __FORECAST_H
#define __FORECAST_H

typedef struct {
	int date;
	int hour;
	int conditions[4];
	char swell_size;
	char *county_name;
	char *spot_name;
} Forecast;

typedef struct {
	int date;
	int hour;
	char tide_height;
	char *county_name;
} TideForecast;

Forecast *create_forecast( char *spot_name, char *county_name, int date, int hour, int general, int swell, int tide, int wind, char swell_size );
TideForecast *create_tide_forecast( char *county_name, int date, int hour, float tide_height );

#endif