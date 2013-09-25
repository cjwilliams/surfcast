#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include "forecast.h"
#include "utils.h"

static uint32_t CONDITION_ICONS[] = {
  RESOURCE_ID_POOR_COND,
  RESOURCE_ID_PF_COND,
  RESOURCE_ID_FAIR_COND,
  RESOURCE_ID_FG_COND,
	RESOURCE_ID_GOOD_COND
};

Forecast forecasts[ 24 ];
TideForecast tide_forecasts[ 24 ];

int next_forecast = 0;
int next_tide_forecast = 0;

Forecast *create_forecast( char *spot_name, char *county_name, int date, int hour, int general, int swell, int tide, int wind, char swell_size ){
	forecasts[ next_forecast ].spot_name = spot_name;
	forecasts[ next_forecast ].county_name = county_name;
	forecasts[ next_forecast ].date = date;
	forecasts[ next_forecast ].hour = hour;
	forecasts[ next_forecast ].conditions[0] = general;
	forecasts[ next_forecast ].conditions[1] = swell;
	forecasts[ next_forecast ].conditions[2] = tide;
	forecasts[ next_forecast ].conditions[3] = wind;
	forecasts[ next_forecast ].swell_size = swell_size;

	return( &forecasts[ next_forecast++ ] );
}

TideForecast *create_tide_forecast( char *county_name, int date, int hour, float tide_height ){
	tide_forecasts[ next_tide_forecast ].county_name = county_name;
	tide_forecasts[ next_tide_forecast ].date = date;
	tide_forecasts[ next_tide_forecast ].hour = hour;
	tide_forecasts[ next_tide_forecast ].tide_height = tide_height;
	
	return( &tide_forecasts[ next_tide_forecast++ ] ); 
}

void print_sample_forecast(){
	Forecast *sample = forecasts;
	
	test_print( sample->spot_name );
}
