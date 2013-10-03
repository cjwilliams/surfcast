#ifndef __UTILS_H
#define __UTILS_H

int get_current_date( void );
int get_current_hour( void );

void set_current_datetime( struct tm *tick_time, TimeUnits units_changed );

void start_timer();
void stop_timer();

int get_current_date( void );
int get_current_hour( void );

void set_stopped_flag();

void debug_reason( AppMessageResult reason );
void debug_dictionary_result( DictionaryResult result );

#endif