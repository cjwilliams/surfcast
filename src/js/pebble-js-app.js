// ==== Pebble Surfcast Application ====
// pebble-js-app.js
//
// This file provides the following functionality:
// * On receipt of forecast request from watch, queries the Spitcast API for forecast data for the duration requested
// * Also queries the Spitcast API for tide height data for the same duration in the counties represented by the chosen spots
// * Sends JSON blobs with forecast and tide height data to watch using Pebble's sendAppMessage
//

// TODO: Double check variable scopes!!!

// ========== GLOBAL VARIABLES ==========
var mySpots = [ 
		{ county: "orange-county", spot_id: 602, spot_name: "Surfside Jetty" },
		{ county: "santa-cruz", spot_id: 2, spot_name: "Steamer Lane" },
		{ county: "san-diego", spot_id: 594, spot_name: "Oceanside Pier" },
		{ county: "san-francisco", spot_id: 697, spot_name: "Kellys Cove" },
		{ county: "monterey", spot_id: 154, spot_name: "Carmel Beach" }
	];
var myCounties = [ "orange-county","santa-cruz","san-diego","san-francisco","monterey" ];
var myForecasts = [];
var myTides = [];
var queuedMessage = {};
var duration = 24;
var datetime = 0;
var date = 0;
var hour = 0;
// ========== GLOBAL VARIABLES ==========

function setCurrentDateTime() {
	datetime = new Date();
	date = datetime.getDate();
	hour = datetime.getHours();
}

// This function uses regex to return the number day of the forecast
function getDayFromString( str ) {
	return( parseInt( str.replace( /^[A-Za-z\s]*/,"" ).replace( /\s[0-9]{4}$/,"" ),10 ) );
}

// This function uses regex to return the hour of the forecast
function getHourFromString( str ) {
	var hour = 0;

	if( str.match( '12AM' ) != null ) { hour = -12; }
	else if( str.match( '12PM' ) != null ) { hour = 0; }
	else if( str.match( /PM$/ ) != null ) { hour = 12; }

	hour += parseInt( str.replace( /[AP][M]$/,"" ),10 );

	return( hour );
}

// This function returns an integer key value equivalent to the provided forecast condition string
function conditionIdFromString( conditionType ) {
	switch( conditionType ) {
  	case "Poor": return 0; break;
		case "Poor-Fair": return 1; break;
		case "Fair": return 2; break;
		case "Fair-Good": return 3; break;
		case "Good": return 4; break;
  }
}

// This function performs the API calls for forecast data & calls sendAppMessage to send the response to the Pebble watch for display
function fetchSpotConditions( spot, duration ) {
	var request = new XMLHttpRequest();
	request.open( 'GET', 'http://api.spitcast.com/api/spot/forecast/'+spot.spot_id.toString()+'/?dcat=week', true );

	request.onload = function( e ) {
		if( request.readyState == 4 && request.status == 200 ) {
			var response = JSON.parse( request.responseText );

			if( response && response.length > 0 ) {
				for( var i=0; i<duration; i++ ) {
					var result = response[ i ];
					var result_date = getDayFromString( result.date );
					var result_hour = getHourFromString( result.hour );
					
					if( result_date >= date || ( result_date === date && result_hour >= hour ) ){
						myForecasts.push({ "spot": result.spot_name.toString(), "county": spot.county, "date": result_date, "hour": result_hour, "general": conditionIdFromString( result.shape_full ), "swell": conditionIdFromString( result.shape_detail.swell ), "tide": conditionIdFromString( result.shape_detail.tide ), "wind": conditionIdFromString( result.shape_detail.wind ), "swell_size": result.size });
					}
				}
			}
		}
		else { console.log( "Error" ); }
	}
	request.send( null );
}

// This function performs the API calls for tide data & calls sendAppMessage to send the response to the Pebble watch for display
function fetchCountyTides( county, duration ) {
	var request = new XMLHttpRequest();
	request.open( 'GET', 'http://api.spitcast.com/api/county/tide/'+county+'/', true );

	request.onload = function( e ) {
		if( request.readyState == 4 && request.status == 200 ) {
			var response = JSON.parse( request.responseText );

			if( response && response.length > 0 ) {
				for( var i=0; i<duration; i++ ) {
					var result = response[ i ];
					var result_date = getDayFromString( result.date );
					var result_hour = getHourFromString( result.hour );
					
					if( result_date >= date || ( result_date === date && result_hour >= hour ) ){
						var height = Math.floor( result.tide*100 );
						myTides.push({ "county": county.toString(), "date": result_date, "hour": result_hour, "tide_height": height });
					}
				}
			}
		}
		else { console.log( "Error" ); }
	}
	request.send( null );
}

// Can optimize this further by allowing param of retrievalDate, to minimize messages to send to watch
// This function facilitates the collection of forecasts and tide heights
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
}

// This function transmits the next forecast/tide data to the Pebble watch
function transmitNextForecast() {
	console.log( "Forecasts: "+myForecasts.length );
	console.log( "Tides: "+myTides.length );
	
	if( myForecasts.length > 0 ) {
		queuedMessage = myForecasts.pop();
		Pebble.sendAppMessage( queuedMessage );
		console.log( "Forecast transmission" );
	}
	else if( myTides.length > 0 ) {
		queuedMessage = myTides.pop();
		Pebble.sendAppMessage( queuedMessage );
		console.log( "Tide transmission" );
	}
	else {
		Pebble.sendAppMessage({ "request_status": 0 })
		console.log( "Empty tides/forecasts" );
	}
}

// This function retransmits forecast/tide data that may not have made it to the Pebble watch
function retransmitForecast() {
	if( Object.keys( queuedMessage ).length > 0 ){
		Pebble.sendAppMessage( queuedMessage );
		console.log( "Resent dropped message" );
	}
	else {
		console.log( "No messages in queue" );
		transmitNextForecast();
	}
}

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
													if( e.payload.request_status == 2 ){ retransmitForecast(); }
													else if( e.payload.request_status == 3 ){ fetchSurfcast( duration ); }
													else { transmitNextForecast(); }
                        });
// Removed "if( myTides.length > 0 || myForecasts.length >0 )" from last else because the extra +1 iteration seems to be necessary to signal the watch app