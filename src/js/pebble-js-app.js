const FORECAST_IO_API_KEY = 'FORECAST_IO_API_KEY';

function getIconId(iconName) {
    return ['clear-day', 'clear-night', 'rain', 'snow', 'sleet', 'wind', 'fog', 'cloudy', 'partly-cloudy-day', 'partly-cloudy-night'].indexOf(iconName);
}

function fetchWeather(latitude, longitude) {
    console.log('getting forecast');
    var request = new XMLHttpRequest();
    var urlBase = 'http://api.forecast.io/forecast/';
    request.open('GET', urlBase + FORECAST_IO_API_KEY + '/' + latitude + ',' + longitude + '?units=auto&exclude=hourly,daily,alerts,flags', true);
    request.onload = function() {
        if (request.readyState === 4) {
            if (request.status === 200) {
                var response = JSON.parse(request.responseText);
                Pebble.sendAppMessage({
                    "icon": getIconId(response['currently']['icon']),
                    "temperature": Math.round(response['currently']['temperature']) + '\u00B0F'
                });
            } else {
                console.log('Error: ' + request.status);
            }
        }
    };
    request.send(null);
}

function locationSuccess(pos) {
    var coordinates = pos.coords;
    fetchWeather(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
    console.warn('location error (' + err.code + '): ' + err.message);
    Pebble.sendAppMessage({
        "city":"Loc Unavailable",
        "temperature":"N/A"
    });
}

var locationOptions = { "timeout": 15000, "maximumAge": 60000 };

Pebble.addEventListener('ready', function(e) {
    window.navigator.geolocation.watchPosition(locationSuccess, locationError, locationOptions);
});

Pebble.addEventListener('appmessage', function(e) {
    window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
});
