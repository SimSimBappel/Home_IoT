# check weather
To make sure the blinds are commanded up everytime the windspeed exceeds 15m/s this script checks via OpenWeatherMap API.
this script is run once per hour in crontab -e using the following line

```
0 * * * * /path/to/python3 /path/to/check_weather.py
```