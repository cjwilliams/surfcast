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
// ========== GLOBAL VARIABLES ==========

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

// This function performs the API calls for forecast data
function fetchSpotConditions( spot, duration ) {
	var request = new XMLHttpRequest();
	request.open( 'GET', 'http://api.spitcast.com/api/spot/forecast/'+spot.spot_id.toString()+'/?dcat=week', true );

	request.onload = function( e ) {
		if( request.readyState == 4 && request.status == 200 ) {
			var response = JSON.parse( request.responseText );

			if( response && response.length > 0 ) {
				for( var i=0; i<duration; i++ ) {
					var result = response[ i ];
					myForecasts.push({ "spot": result.spot_name.toString(), "county": spot.county, "date": getDayFromString( result.date ), "hour": getHourFromString( result.hour ), "general": conditionIdFromString( result.shape_full ), "swell": conditionIdFromString( result.shape_detail.swell ), "tide": conditionIdFromString( result.shape_detail.tide ), "wind": conditionIdFromString( result.shape_detail.wind ), "swell_size": result.size });
				}
			}
		}
		else { console.log( "Error" ); }
	}
	request.send( null );
}

// This function performs the API calls for tide data
function fetchCountyTides( county, duration ) {
	var request = new XMLHttpRequest();
	request.open( 'GET', 'http://api.spitcast.com/api/county/tide/'+county+'/', true );

	request.onload = function( e ) {
		if( request.readyState == 4 && request.status == 200 ) {
			var response = JSON.parse( request.responseText );

			if( response && response.length > 0 ) {
				for( var i=0; i<duration; i++ ) {
					var result = response[ i ];
					var height = Math.floor( result.tide*100 );
					myTides.push({ "county": county.toString(), "date": getDayFromString( result.date ), "hour": getHourFromString( result.hour ), "tide_height": height });
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
		
	for( var i=0; i<mySpots.length; i++ ){
		fetchSpotConditions( mySpots[ i ],duration );
	}

	for( var j=0; j<myCounties.length; j++ ){
		fetchCountyTides( myCounties[ j ],duration );
	}
}

// This function transmits the next forecast/tide data to the Pebble watch
function transmitNextForecast() {
	console.log( "Forecasts: "+myForecasts.length );
	console.log( "Tides: "+myTides.length );
	
	if( myForecasts.length > 0 ) {
		queuedMessage = myForecasts.pop();
		console.log( "Forecast transmission" );
		Pebble.sendAppMessage( queuedMessage, function( e ){ transmitNextForecast(); }, function( e ){ console.log( e.error.message ); retransmitForecast(); } );
	}
	else if( myTides.length > 0 ) {
		queuedMessage = myTides.pop();
		console.log( "Tide transmission" );
		Pebble.sendAppMessage( queuedMessage, function( e ){ transmitNextForecast(); }, function( e ){ console.log( e.error.message ); retransmitForecast(); } );
	}
	else {
		console.log( "Empty tides/forecasts" );
	}
}

// This function retransmits forecast/tide data that may not have made it to the Pebble watch
function retransmitForecast() {
	if( Object.keys( queuedMessage ).length > 0 ){
		console.log( "Resent dropped message" );
		Pebble.sendAppMessage( queuedMessage, function( e ){ transmitNextForecast(); }, function( e ){ console.log( e.error.message ); retransmitForecast(); } );
	}
	else {
		console.log( "No messages in queue" );
		transmitNextForecast();
	}
}

// This listener directs the preparation for forecast requests
Pebble.addEventListener( "ready",
                        function( e ) {
                          console.log( "connect!" + e.ready );
													fetchSurfcast( duration ); 
													console.log( e.type );
                        });
												
Pebble.addEventListener( "showConfiguration",
												function( e ) {
													console.log( e.type );
													Pebble.openURL( "http://surfcast.s3-website-us-east-1.amazonaws.com" );
												});

Pebble.addEventListener( "webviewclosed",
											  function( e ) {
													var configuration = JSON.parse( decodeURIComponent( e.response ) );
													console.log( "Configuration returned: " + JSON.stringify( configuration ) );
												});

// This listener directs the forecast request & response activities
Pebble.addEventListener( "appmessage",
                        function( e ) {
                          console.log( e.type );
													console.log( e.payload.request_status );
													console.log( "message!" );
													if( e.payload.request_status == 0 ){ return; }
													else if( e.payload.request_status == 1 ){ transmitNextForecast(); }
													else if( e.payload.request_status == 2 ){ fetchSurfcast(); }
                        });