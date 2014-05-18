/*global Pebble*/

const FORECAST_IO_API_KEY = '';

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
                    'icon': getIconId(response['currently']['icon']),
                    'temperature': Math.round(response['currently']['temperature']) + '\u00B0F',
                    'message': ''
                });
            } else {
                console.log('Error: ' + request.status);
            }
        }
    };
    request.send(null);
}

function locationSuccess(pos) {
    fetchWeather(pos.coords.latitude, pos.coords.longitude);
}

var locationOptions = { "timeout": 15000, "maximumAge": 600000 };

function locationError(err) {
    console.warn('location error (' + err.code + '): ' + err.message);
    switch(err.code) {
        case err.TIMEOUT:
            // Quick fallback when no suitable cached position exists.
            Pebble.sendAppMessage({
                'message': 'Location Timed Out'
            });
            // Acquire a new position object.
            navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
            break;
        case err.PERMISSION_DENIED:
            Pebble.sendAppMessage({
                'message': 'Loc Permission Denied'
            });
            break;
        default: // POSITION_UNAVAILABLE
            Pebble.sendAppMessage({
                'message': 'Location Unavailable'
            });
    }
}

Pebble.addEventListener('ready', function(e) {
    function getForecastForCurrentLocation() {
      window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
    }
    getForecastForCurrentLocation();
    setInterval(getForecastForCurrentLocation, 15 * 60 * 1000);
});
