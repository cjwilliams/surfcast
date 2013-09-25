#ifndef __UTILS_H
#define __UTILS_H

void start_timer();
void stop_timer();

void set_stopped_flag();
void reset_stopped_flag();

void test_print( char *str );
void debug_reason( AppMessageResult reason );
void debug_dictionary_result( DictionaryResult result );

#endif