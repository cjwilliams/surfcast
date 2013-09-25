function conditionIdFromString( conditionType ) {
	switch( conditionType ) {
  	case "Poor": return 0;
		case "Poor-Fair": return 1;
		case "Fair": return 2;
		case "Fair-Good": return 3;
		case "Good": return 4;
  }
}

function fetchSurfConditions() {
	var request = new XMLHttpRequest();
	request.open( 'GET', 'http://api.spitcast.com/api/spot/forecast/1/', true );
	request.onload = function( e ) {
		if ( request.readyState == 4 && request.status == 200 ) {
			var response = JSON.parse( request.responseText );
			console.log( conditionIdFromString( response[0].shape_full ), conditionIdFromString( response[0].shape_detail.swell ), conditionIdFromString( response[0].shape_detail.tide ), conditionIdFromString( response[0].shape_detail.wind ), response[0].size, response[0].spot_name, response[0].gmt );

				// var conditions = { conditionIdFromString( response[0].shape_full ), conditionIdFromString( response[0].shape_detail.swell ), conditionIdFromString( response[0].shape_detail.tide ), conditionIdFromString( response[0].shape_detail.wind ), response[0].size };
			Pebble.sendAppMessage({ "time": response[0].gmt, "location": response[0].spot_name, "overall": conditionIdFromString( response[0].shape_full ) });				}
		else {
			console.log("Error");
		}
	}
  request.send(null);
}


PebbleEventListener.addEventListener( "ready",
                        function( e ) {
                          console.log( "connect!" + e.ready );
                          fetchSurfConditions();
                          console.log( e.type );
                        });

PebbleEventListener.addEventListener( "appmessage",
                        function( e ) {
                          console.log( e.type );
                          console.log( e.payload.location );
                          console.log( "message!" );
                          fetchSurfConditions();
                        });

PebbleEventListener.addEventListener("webviewclosed",
                                     function(e) {
                                     console.log("webview closed");
                                     console.log(e.type);
                                     console.log(e.response);
                                     });

// Pebble.sendAppMessage({ "icon": ICON_SUNNY, "temperature": "28oC"});
// Pebble.showSimpleNotificationOnPebble(title, text)