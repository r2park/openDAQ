Gmaps = QWebView();
        html = '''
        <!DOCTYPE html>
        <html>

        <head>
            <meta name="viewport" content="initial-scale=1.0, user-scalable=no" />
            <style type="text/css">
                html {
                    height: 100%
                }
                body {
                    height: 100%;
                    margin: 0;
                    padding: 0
                }
                #map_canvas {
                    height: 100%
                }
            </style>
            <script type="text/javascript" src="http://maps.googleapis.com/maps/api/js?sensor=false">
            </script>
            <script type="text/javascript">
            var markers = [];
            var map;

            function Tour_startUp(stops) {
                if (!window.tour) window.tour = {
                    updateStops: function(newStops) {
                        stops = newStops;
                    },
                    loadMap: function(map, directionsDisplay) {
                        var myOptions = {
                            zoom: 15,
                            center: new window.google.maps.LatLng(50.9801, 4.97517),
                            mapTypeId: window.google.maps.MapTypeId.ROADMAP
                        };
                        map.setOptions(myOptions);
                        directionsDisplay.setMap(map);
                    },
                    fitBounds: function(map) {
                        var bounds = new window.google.maps.LatLngBounds();
                        jQuery.each(stops, function(key, val) {
                            var myLatlng = new window.google.maps.LatLng(val.Geometry.Latitude, val.Geometry.Longitude);
                            bounds.extend(myLatlng);
                        });
                        map.fitBounds(bounds);
                    },
                        calcRoute: function(directionsService, directionsDisplay) {
                            var batches = [];
                            var itemsPerBatch = 10;
                            var itemsCounter = 0;
                            var wayptsExist = stops.length > 0;
                            while (wayptsExist) {
                                var subBatch = [];
                                var subitemsCounter = 0;
                                for (var j = itemsCounter; j < stops.length; j++) {
                                   subitemsCounter++;
                                   subBatch.push({
                                       location: new window.google.maps.LatLng(stops[j].Geometry.Latitude, stops[j].Geometry.Longitude),
                                       stopover: true
                                   });
                                   if (subitemsCounter == itemsPerBatch) break;
                               }
                               itemsCounter += subitemsCounter;
                               batches.push(subBatch);
                               wayptsExist = itemsCounter < stops.length;
                               itemsCounter--;
                           }
                           var combinedResults;
                           var unsortedResults = [{}];
                           var directionsResultsReturned = 0;
                           for (var k = 0; k < batches.length; k++) {
                               var lastIndex = batches[k].length - 1;
                               var start = batches[k][0].location;
                               var end = batches[k][lastIndex].location;
                               var waypts = [];
                               waypts = batches[k];
                               waypts.splice(0, 1);
                               waypts.splice(waypts.length - 1, 1);
                               var request = {
                                   origin: start,
                                   destination: end,
                                   waypoints: waypts,
                                   travelMode: window.google.maps.TravelMode.WALKING
                               };
                               (function(kk) {
                                   directionsService.route(request, function(result, status) {
                                       if (status == window.google.maps.DirectionsStatus.OK) {
                                           var unsortedResult = {
                                               order: kk,
                                               result: result
                                           };
                                           unsortedResults.push(unsortedResult);
                                           directionsResultsReturned++;
                                           if (directionsResultsReturned == batches.length) {
                                               unsortedResults.sort(function(a, b) {
                                                   return parseFloat(a.order) - parseFloat(b.order);
                                               });
                                               var count = 0;
                                               for (var key in unsortedResults) {
                                                   if (unsortedResults[key].result != null) {
                                                       if (unsortedResults.hasOwnProperty(key)) {
                                                           if (count == 0) combinedResults = unsortedResults[key].result;
                                                           else {
                                                               combinedResults.routes[0].legs = combinedResults.routes[0].legs.concat(unsortedResults[key].result.routes[0].legs);
                                                               combinedResults.routes[0].overview_path = combinedResults.routes[0].overview_path.concat(unsortedResults[key].result.routes[0].overview_path);
                                                               combinedResults.routes[0].bounds = combinedResults.routes[0].bounds.extend(unsortedResults[key].result.routes[0].bounds.getNorthEast());
                                                               combinedResults.routes[0].bounds = combinedResults.routes[0].bounds.extend(unsortedResults[key].result.routes[0].bounds.getSouthWest());
                                                           }
                                                           count++;
                                                       }
                                                   }
                                               }
                                               directionsDisplay.setDirections(combinedResults);
                                           }
                                       }
                                   });
                               })(k);
                           }
                       }
                   };
               }

               function initialize() {
                   var stops = [{
                       "Geometry": {
                           "Latitude": 50.9799,
                           "Longitude": 4.97229
                       }
                   }, {
                       "Geometry": {
                           "Latitude": 50.8069,
                           "Longitude": 4.9214
                       }
                   }, {
                       "Geometry": {
                           "Latitude": 50.8639,
                           "Longitude": 4.69594
                       }
                   }, {
                       "Geometry": {
                           "Latitude": 50.7226,
                           "Longitude": 4.62634
                       }
                   }, {
                       "Geometry": {
                           "Latitude": 50.7541,
                           "Longitude": 4.98607
                       }
                   }, {
                       "Geometry": {
                           "Latitude": 50.637,
                           "Longitude": 5.90936
                       }
                   }, {
                       "Geometry": {
                           "Latitude": 50.9463,
                           "Longitude": 5.69475
                       }
                   }, {
                       "Geometry": {
                           "Latitude": 50.9499,
                           "Longitude": 5.484
                       }
                   }, {
                       "Geometry": {
                           "Latitude": 50.9799,
                           "Longitude": 4.97229
                       }
                   }];
                   var myOptions = {
                       center: new google.maps.LatLng(50.9801, 4.97517),
                       zoom: 15,
                       mapTypeId: google.maps.MapTypeId.ROADMAP,
                       panControl: true
                   };
                   map = new window.google.maps.Map(document.getElementById("map_canvas"), myOptions);
                   var directionsDisplay = new window.google.maps.DirectionsRenderer();
                   var directionsService = new window.google.maps.DirectionsService();
                   Tour_startUp(stops);
                   window.tour.loadMap(map, directionsDisplay);
                   if (stops.length > 1) window.tour.calcRoute(directionsService, directionsDisplay);
                };
            </script>
        </head>

        <body onLoad='initialize()'>
            <div id="map_canvas" style="width:100%;height:100%"> </div>
        </body>

        </html>
        '''

        Gmaps.setHtml(html)