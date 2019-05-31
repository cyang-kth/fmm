/**
 *   FMM web application
 *   Author: Can Yang 
 */
var center = [59.3293, 18.0686];
var zoom_level = 11;
map = new L.Map('map', {
    center: new L.LatLng(center[0], center[1]),
    zoom: zoom_level
});
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
}).addTo(map);

var matched_result_layer;
var current_drawing_data;

var editableLayers = new L.FeatureGroup();
map.addLayer(editableLayers);

var options = {
    position: 'topleft',
    draw: {
        polyline: {
            shapeOptions: {
                color: '#996633', //f357a1
                weight: 4
            },
            repeatMode: false,
            // Increase the icon size
            icon: new L.DivIcon({
                iconSize: new L.Point(20, 20),
                className: 'leaflet-div-icon leaflet-editing-icon my-own-icon'
            }),
        },
        polygon: false,
        circle: false, // Turns off this drawing tool
        rectangle: false,
        marker: false,
    },
    edit: false,
};
var drawControl = new L.Control.Draw(options);
map.addControl(drawControl);

map.on(L.Draw.Event.CREATED, function(e) {
    var type = e.layerType,
        layer = e.layer;
    editableLayers.addLayer(layer);
    // Run map matching
    var traj = e.layer.toGeoJSON();
    var wkt = Terraformer.WKT.convert(traj.geometry);
    console.log(wkt)
    match_wkt(wkt);
});

var match_wkt = function(wkt) {
    $.getJSON("/match_wkt", {
            "wkt": wkt
        }).done(function(data) {
            console.log("Result fetched");
            console.log(data);
            if (data.state==1){
                var geojson = Terraformer.WKT.parse(data.wkt);
                var geojson_layer = L.geoJson(
                    geojson,
                    {
                        style: function(feature) {
                            return {
                                color: 'red'
                            };
                        }
                    }
                );
                editableLayers.addLayer(geojson_layer);
            } else {
                alert("Cannot match the trajectory, try another one")
            }
        })
        .fail(function(jqXHR, textStatus, errorThrown) {
            alert("Error with fetching data from server");
            console.log("error " + textStatus);
            console.log("incoming Text " + jqXHR.responseText);
        }).always(function() {
            console.log("complete");
        });
}

map.on(L.Draw.Event.DRAWSTART, function(e) {
    editableLayers.clearLayers();
});

L.Control.RemoveAll = L.Control.extend({
    options: {
        position: 'topleft',
    },
    onAdd: function(map) {
        var controlDiv = L.DomUtil.create('div', 'leaflet-control leaflet-bar leaflet-draw-toolbar');
        var controlUI = L.DomUtil.create('a', 'leaflet-draw-edit-remove', controlDiv);
        // var icon = L.DomUtil.create('span', 'fa fa-trash', controlDiv);
        // icon.setAttribute('aria-hidden',true);
        controlUI.title = 'Clean map';
        controlUI.setAttribute('href', '#');
        L.DomEvent
            .addListener(controlUI, 'click', L.DomEvent.stopPropagation)
            .addListener(controlUI, 'click', L.DomEvent.preventDefault)
            .addListener(controlUI, 'click', function() {
                if (editableLayers.getLayers().length == 0) {
                    alert("No features drawn");
                } else {
                    editableLayers.clearLayers();
                    $("#uv-div").empty();
                    //chart.destroy();
                }
            });
        return controlDiv;
    }
});
removeAllControl = new L.Control.RemoveAll();
map.addControl(removeAllControl);

var add_listeners = function() {
    $('#zoom_center').click(function() {
        map.setView(center, zoom_level);
    });
    $('#clean_map').click(function() {
        editableLayers.clearLayers();
    });
};
add_listeners();

var wkt2geojson = function(data) {
    // Generate a MultiLineString
    var multilinestring_json = Terraformer.WKT.parse(data);
    var coordinates = multilinestring_json.coordinates;
    var result = {
        "type": "FeatureCollection",
        "features": []
    };
    var arrayLength = coordinates.length;
    for (var i = 0; i < arrayLength; i++) {
        result.features.push({
            "type": "Feature",
            "properties": {},
            "geometry": {
                "type": "LineString",
                "coordinates": coordinates[i],
            }
        });
    }
    return result;
};
