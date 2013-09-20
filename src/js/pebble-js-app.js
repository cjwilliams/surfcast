function conditionIdFromString( conditionType ) {
	switch( conditionType ) {
  	case "Poor": return 0;
		case "Poor-Fair": return 1;
		case "Fair": return 2;
		case "Fair-Good": return 3;
		case "Good": return 4;
  }
}

function getDayFromString( str ) {
	return( parseInt( str.replace( /^[A-Za-z\s]*/,"" ).replace( /\s[0-9]{4}$/,"" ),10 ) );
}

function getHourFromString( str ) {
	var hour = 0;
	if( str.match( '12AM' ) != null ) {
		hour = -12;
	}
	else if( str.match( '12PM' ) != null ) {
		hour = 0;
	}
	else if( str.match( /PM$/ ) != null ) {
		hour = 12;
	}
	hour += parseInt( str.replace( /[AP][M]$/,"" ),10 );
	
	return( hour );
}

function fetchSurfcast( spotId, duration ) {
  var request = new XMLHttpRequest();
	request.open( 'GET', 'http://api.spitcast.com/api/spot/forecast/'+spotId.toString()+'/?dcat=week', true );
  request.onload = function( e ) {
    if ( request.readyState == 4 ) {
      if( request.status == 200 ) {
        var response = JSON.parse( request.responseText );
				
        if ( response && response.length > 0 ) {
          var data = [];

					for( var i=0; i<duration; i++ ) {
						var result = response[ i ];
						console.log( [ result.spot_id, duration ] )
						var array = [ getDayFromString( result.date ), getHourFromString( result.hour ), conditionIdFromString( result.shape_full ), conditionIdFromString( result.shape_detail.swell ), conditionIdFromString( result.shape_detail.tide ), conditionIdFromString( result.shape_detail.wind ), result.size ];
						if( i%12 == 0 ) {
							Pebble.sendAppMessage({ "location":result.spot_id, "data":data });
							data.length = 0;
						}
						data.push( array );
						console.log( array );
					}
					Pebble.sendAppMessage({ "location":result.spot_id, "data":data });	
        }

      } else {
        console.log( "Error" );
      }
    }
  }
  request.send( null );
}


PebbleEventListener.addEventListener( "ready",
                        function( e ) {
                          console.log( "connect!" + e.ready );
                          console.log( e.type );
                        });

PebbleEventListener.addEventListener( "appmessage",
                        function( e ) {
                          console.log( e.type );
													console.log( "message!" );
                          fetchSurfcast( e.payload.location, e.payload.limit );
                        });

PebbleEventListener.addEventListener( "webviewclosed",
                                     function(e) {
                                     console.log( "webview closed" );
                                     console.log( e.type );
                                     console.log( e.response );
                                     });