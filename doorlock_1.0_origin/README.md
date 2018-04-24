Add crontab for regular execution

# vi /etc/crontab
	HOME = /home/pi/

	0 * /3 * * * root python3 /HOMEIT/weather_tts_api/weather_tts_api.py

# service cron start
-----------------------------------------------------------------------
automatically execute [weather_tts_api.py] every 3hours
|
V
make [weather_notice.mp3]
-----------------------------------------------------------------------
doorlock app's INSIDE_BTN interrupt
|
V
call [playSound.sh]
|
V
play [weather_notice.mp3]

