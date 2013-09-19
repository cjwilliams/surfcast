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
	day = str.replace( /\d{4}\-\d(\d)?\-/,"" ).replace( /\s\d(\d)?$/,"" );
	return( parseInt( day,10 ) );
}

function getHourFromString( str ) {
	hour = str.replace( /\d{4}(\-\d(\d)?){2}\s/,"" );
	return( parseInt( hour,10 ) - 7 );
}

function fetchSurfcast( spotId, duration ) {
  var request = new XMLHttpRequest();
	request.open( 'GET', 'http://api.spitcast.com/api/spot/forecast/'+spotId.toString()+'/', true );
  request.onload = function( e ) {
    if ( request.readyState == 4 ) {
      if( request.status == 200 ) {
        console.log( request.responseText );
        var response = JSON.parse( request.responseText );

        var condition, icon;

				if ( response && response.length > 0 ) {
          for( var i=0; i<duration; i++ ) {
						var result = response[ i ];
						var array = [ getDayFromString( result.gmt ), getHourFromString( result.gmt ), conditionIdFromString( result.shape_full ), conditionIdFromString( result.shape_detail.swell ), conditionIdFromString( result.shape_detail.tide ), conditionIdFromString( result.shape_detail.wind ), result.size ];
						console.log( array );
						Pebble.sendAppMessage({ "location":result.spot_id, "data":array });
					}
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