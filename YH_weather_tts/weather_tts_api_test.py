#########################################################################################################################################
#	2018.04.05															#
#																	#
#	+ 생성된 .mp3파일 실행														#
#																	#
# weather api serviceKey : 	"--"	#
#																	#
# nx,ny : 61, 125 (서초2동)														#
#																	#
# TTS api 			client_id = "--"         								#
#				client_secret = "--"										#
#																	#
# get_api_date() 	현재 시간 반환,													#
# get_weather_data() 	현재 시간의 특정 지역(서초2동) 강수확률 반환,									#
# get_mp3file(text) 	선택된 알림말 text를 인자로 받아 text를 말하는 mp3파일 생성							#
#																	#
# # vi /etc/crontab		지정된 시간마다 실행하기 위함										#
# 	* * * * * root python3 /HOMIT/weather_tts_api/weather_tts_api.py								#
# # service cron start															#
#########################################################################################################################################

import datetime
import pytz
import urllib.request
import json
import string
import sys
import time
import os
#from pygame import mixer 
#mixer.~

def get_api_date() :
	standard_time = [2, 5, 8, 11, 14, 17, 20, 23]
	time_now = datetime.datetime.now(tz=pytz.timezone('Asia/Seoul')).strftime('%H')
	check_time = int(time_now) - 1
	day_calibrate = 0
	while not check_time in standard_time :
		check_time -= 1
		if check_time < 2 :
			day_calibrate = 1
			check_time = 23

	date_now = datetime.datetime.now(tz=pytz.timezone('Asia/Seoul')).strftime('%Y%m%d')
	check_date = int(date_now) - day_calibrate

	if check_time < 10:
		return (str(check_date), ('0' + str(check_time) + '00'))
	else:	
		return (str(check_date), (str(check_time) + '00'))

def get_weather_data() :
	api_date, api_time = get_api_date()
	url = "http://newsky2.kma.go.kr/service/SecndSrtpdFrcstInfoService2/ForecastSpaceData?"
	key = "serviceKey=" + "--" 
	date = "&base_date=" + api_date
	time = "&base_time=" + api_time
	nx = "&nx=61"
	ny = "&ny=125"
	numOfRows = "&numOfRows=100"
	type = "&_type=json"
	api_url = url + key + date + time + nx + ny + numOfRows + type

#	print(api_url)
#	print()

	data = urllib.request.urlopen(api_url).read().decode('utf8')
	data_json = json.loads(data)
	
#	print(data_json)
#	print()

	parsed_json = data_json['response']['body']['items']['item']

	target_date = parsed_json[0]['fcstDate']  # get date and time
	target_time = parsed_json[0]['fcstTime']

	date_calibrate = target_date #date of TMX, TMN
	if target_time > 1300:
		date_calibrate = str(int(target_date) + 1)

	passing_data = {}
	for one_parsed in parsed_json:
		if one_parsed['fcstDate'] == target_date and one_parsed['fcstTime'] == target_time: #get today's data
			passing_data[one_parsed['category']] = one_parsed['fcstValue']

		if one_parsed['fcstDate'] == date_calibrate and (one_parsed['category'] == 'TMX' or one_parsed['category'] == 'TMN'): #TMX, TMN at calibrated day
			passing_data[one_parsed['category']] = one_parsed['fcstValue']

		if one_parsed['category'] == 'POP':					#return POP value
                          return one_parsed['fcstValue']

#	return passing_data

def get_mp3file(text) :
	client_id = "--"
	client_secret = "--"

	encText = urllib.parse.quote(text)
	data = "speaker=mijin&speed=0&text=" + encText;
	url = "https://openapi.naver.com/v1/voice/tts.bin"
	request = urllib.request.Request(url)
	request.add_header("X-Naver-Client-Id",client_id)
	request.add_header("X-Naver-Client-Secret",client_secret)
	response = urllib.request.urlopen(request, data=data.encode('utf-8'))
	rescode = response.getcode()
	if(rescode==200):
		print("TTS mp3 저장")
		response_body = response.read()
		with open('weather_notice.mp3', 'wb') as f:
			f.write(response_body)
#			playSound("weather_notice.mp3")
	else:
		print("Error Code:" + rescode)

#play mp3file using Library
#def playSound(ttsSound):
#	mixer.init()
#	mixer.music.load(ttsSound)
#	mixer.music.play()
#	time.sleep(3)


if __name__ == '__main__':
	data = get_weather_data()
	print(data)

	if data <= 30:
		text = "우산은\ 필요없을지\ 몰라요"
		print(text)
	elif data <= 60:
		text = "우산을\ 챙기는게\ 어떨까요"
		print(text)
	else:
		text = "우산\ 반드시\ 챙기세요"
		print(text)

	get_mp3file(text)
	#call TTS function (notice Text)

	#os.system("omxplayer weather_notice.mp3")
	#time.sleep(3)
        #play mp3file

#	print(sys.path)
