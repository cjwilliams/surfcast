typedef struct {
	char *location;
	uint8_t overall;
	uint8_t swell;
	uint8_t tide;
	uint8_t wind;
	float size;
} Forecast;

extern Forecast forecasts[];