// ==== Pebble Surfcast Application ====
// pebble-js-app.js
//
// This file provides the following functionality:
// * On receipt of forecast request from watch, queries the Spitcast API for forecast data for the duration requested
// * Also queries the Spitcast API for tide height data for the same duration in the counties represented by the chosen spots
// * Sends JSON blobs with forecast and tide height data to watch using Pebble's sendAppMessage
//

// TODO: Double check variable scopes!!!

//========== GLOBAL VARIABLES ==========
var MAX_MESSAGE_BYTES = 77;
var mySpots = [ 
		{ spitcast_spot_id: 602, spot_name: "Surfside Jetty", internal_id: 0, county_id: 0 },
		{ spitcast_spot_id: 2, spot_name: "Steamer Lane", internal_id: 1, county_id: 1 },
		{ spitcast_spot_id: 594, spot_name: "Oceanside Pier", internal_id: 2, county_id: 2 },
		{ spitcast_spot_id: 697, spot_name: "Kellys Cove", internal_id: 3, county_id: 3 },
		{ spitcast_spot_id: 154, spot_name: "Carmel Beach", internal_id: 4, county_id: 4 }
	];
var myCounties = [ 
		{ spitcast_county_id: "orange-county", county_name: "Orange County", internal_id: 0 },
		{ spitcast_county_id: "santa-cruz", county_name: "Santa Cruz", internal_id: 1 },
		{ spitcast_county_id: "san-diego", county_name: "San Diego", internal_id: 2 },
		{ spitcast_county_id: "san-francisco", county_name: "San Francisco", internal_id: 3 },
		{ spitcast_county_id: "monterey", county_name: "Monterey", internal_id: 4 }
	];
var myForecasts = [];
var myTides = [];
var myStringLocations = [];
var myStringCounties = [];
var myMessages = [];
var queuedMessage = {};
var current_timestamp = 0;
//========== GLOBAL VARIABLES ==========

function setCurrentDateTime() {
	var datetime = new Date();
	current_timestamp = ( ( ( datetime.getDate() & 0x1F )<<5 | ( datetime.getHours() & 0x1F ) ) & 0x3FF );
}	// 0x1F is a 5bit mask, 0x3FF is a 10bit mask

//========== Massaging Original Data ==========
function getDayFromString( str ) {
	return( parseInt( str.replace( /^[A-Za-z\s]*/,"" ).replace( /\s[0-9]{4}$/,"" ),10 ) );
}	// This function uses regex to return the number day of the forecast

function getHourFromString( str ) {
	var hour = 0;

	if( str.match( '12AM' ) != null ) { hour = -12; }
	else if( str.match( '12PM' ) != null ) { hour = 0; }
	else if( str.match( /PM$/ ) != null ) { hour = 12; }

	hour += parseInt( str.replace( /[AP][M]$/,"" ),10 );

	return( hour );
}	// This function uses regex to return the hour of the forecast

function conditionIdFromString( conditionType ) {
	switch( conditionType ) {
  	case "Poor": return 0; break;
		case "Poor-Fair": return 1; break;
		case "Fair": return 2; break;
		case "Fair-Good": return 3; break;
		case "Good": return 4; break;
  }
}	// This function returns an integer key value equivalent to the provided forecast condition string

//========== Packing Data ==========
function packTimestamp( date_str, time_str ){
	return( ( ( getDayFromString( date_str ) & 0x1F )<<5 | ( getHourFromString( time_str ) & 0x1F ) ) & 0x3FF );
}

// Check mask values!
function packForecast( spot_id, general_str, swell_str, tide_str, wind_str, swell_size ){
	// 24 bits = 3 bytes total
	// Bits 0-7: Spot ID										( spot_id & 0xFF ) << 16
	// Bits 8-10: General Forecast					( general & 0x7 ) << 13
	// Bits 11-13: Swell Forecast						( swell & 0x7 ) << 10
	// Bits 14-16: Tide Forecast						( tide & 0x7 ) << 7
	// Bits 17-19: Wind Forecast						( wind & 0x7 ) << 4
	// Bits 20-23: Swell Size Forecast			( swell_size & 0xF )
	return ( 
		( spot_id & 0xFF )<<16 | 
		( conditionIdFromString( general_str ) & 0x7 )<<13 | 
		( conditionIdFromString( swell_str ) & 0x7 )<<10 | 
		( conditionIdFromString( tide_str ) & 0x7 )<<7 | 
		( conditionIdFromString( wind_str ) & 0x7 )<<4 | 
		( swell_size & 0xF ) 
	);
}

function packForecastHour( timestamp, forecasts ){
	// 2 byte date/hour header + 3 bytes per forecast
	for( var i=0; i<forecasts.length; i++ ){
		timestamp<<=0x100 | ( forecasts[ i ] & 0x13 );	// Check mask values!
	}
	
	return forecast_hour;
}

function stringifyLocations(){
	for( var i=0; i<mySpots.length; i++ ){
		spot = mySpots[ i ];
		console.log( spot.spitcast_spot_id.toString()+spot.internal_id.toString()+spot.county_id.toString()+spot.spot_name );
		myStringLocations.push( spot.spitcast_spot_id.toString()+spot.internal_id.toString()+spot.county_id.toString()+spot.spot_name );
	}
}

function stringifyCounties(){
	for( var i=0; i<myCounties.length; i++ ){
		county = myCounties[ i ];
		console.log( county.internal_id.toString()+county.spitcast_county_id.toString() );
		myStringCounties.push( county.internal_id.toString()+county.spitcast_county_id.toString() );
	}
}

function packMessages(){
	var queuedLocation;
	
	while( myStringLocations.length > 0 ){
		queuedLocation = myStringLocations.pop();
		
		if( ( message.length + queuedLocation.length ) <= MAX_MESSAGE_BYTES ){
			message += queuedLocation;
		}
		else{
			myMessages.push( "locations": message );
			message = queuedLocation;
		}
	}
	
	while( myStringCounties.length > 0 ){
		queuedCounty = myStringCounties.pop();
		
		if( ( message.length + queuedCounty.length ) <= MAX_MESSAGE_BYTES ){
			message += queuedCounty;
		}
		else{
			myMessages.push( "counties": message );
			message = queuedCounty;
		}
	}	
}

function convertToLittleEndian( array ){
	// Is this needed?
}

//========== Retrieving Data ==========
function fetchSpotConditions( spot, duration ) {
	var request = new XMLHttpRequest();
	request.open( 'GET', 'http://api.spitcast.com/api/spot/forecast/'+spot.spitcast_spot_id.toString()+'/?dcat=week', true );

	request.onload = function( e ) {
		if( request.readyState == 4 && request.status == 200 ) {
			var response = JSON.parse( request.responseText );

			if( response && response.length > 0 ) {
				for( var i=0; i<duration; i++ ) {
					var result = response[ i ];
					var timestamp = packTimestamp( result.date, result.hour );
					
					if( timestamp >= current_timestamp ){
						myForecasts.push([ timestamp, packForecast( spot.internal_id, result.shape_full, result.shape_detail.swell, result.shape_detail.tide, result.shape_detail.wind, result.size ) ]);
						myForecasts.sort( function( a,b ){ return a.timestamp-b.timestamp; });
					}
				}
			}
		}
		else { console.log( "Error" ); }
	}
	request.send( null );
} // This function performs the API calls for forecast data & calls sendAppMessage to send the response to the Pebble watch for display

function fetchCountyTides( county, duration ) {
	var request = new XMLHttpRequest();
	request.open( 'GET', 'http://api.spitcast.com/api/county/tide/'+county.spitcast_county_id+'/', true );

	request.onload = function( e ) {
		if( request.readyState == 4 && request.status == 200 ) {
			var response = JSON.parse( request.responseText );

			if( response && response.length > 0 ) {
				for( var i=0; i<duration; i++ ) {
					var result = response[ i ];
					var timestamp = ( getDayFromString( result.date ) & 0x1F )<<5 | ( getHourFromString( result.hour ) & 0x1F );

					if( timestamp >= current_timestamp ){
						var height = Math.floor( result.tide*100 );
						myTides.push({ "county": county.internal_id, "timestamp": timestamp, "tide_height": height });
						myTides.sort( function( a,b ){ return a.timestamp-b.timestamp; });
					}
				}
			}
		}
		else { console.log( "Error" ); }
	}
	request.send( null );
}	// This function performs the API calls for tide data & calls sendAppMessage to send the response to the Pebble watch for display

// Can optimize this further by allowing param of retrievalDate, to minimize messages to send to watch
function fetchSurfcast( duration ) {
	myForecasts = [];
	myTides = [];
	
	setCurrentDateTime();
		
	for( var i=0; i<mySpots.length; i++ ){
		fetchSpotConditions( mySpots[ i ],duration );
	}

	for( var j=0; j<myCounties.length; j++ ){
		fetchCountyTides( myCounties[ j ],duration );
	}
	
	Pebble.sendAppMessage({ "request_status": 4 })
}	// This function facilitates the collection of forecasts and tide heights


//========== Transmitting Data ==========
function transmitNextMessage() {
	if( myMessages.length > 0 ) {
		queuedMessage = myMessages.pop();
		Pebble.sendAppMessage( queuedMessage );
		console.log( "Forecast transmission" );
	}
	else {
		Pebble.sendAppMessage({ "request_status": 0 })
		console.log( "Empty tides/forecasts" );
	}
} // This function transmits the next message to the Pebble watch

function retransmitMessage() {
	if( Object.keys( queuedMessage ).length > 0 ){
		Pebble.sendAppMessage( queuedMessage );
		console.log( "Resent dropped message" );
	}
	else {
		console.log( "No messages in queue" );
		transmitNextMessage();
	}
}	// This function retransmits messages that may not have made it to the Pebble watch




//========== Event Listeners ==========

// This listener directs the preparation for forecast requests
PebbleEventListener.addEventListener( "ready",
                        function( e ) {
                          console.log( "connect!" + e.ready );
													fetchSurfcast( duration );
													console.log( e.type );
                        });

// This listener directs the forecast request & response activities
PebbleEventListener.addEventListener( "appmessage",
                        function( e ) {
                          console.log( e.type );
													console.log( e.payload.request_status );
													console.log( "message!" );
													if( e.payload.request_status == 2 ){ retransmitMessage(); }
													else if( e.payload.request_status == 3 ){ fetchSurfcast( 24 ); }
													else { transmitNextMessage(); }
                        });
// Removed "if( myTides.length > 0 || myForecasts.length >0 )" from last else because the extra +1 iteration seems to be necessary to signal the watch app