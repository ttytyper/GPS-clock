/****
 * A clock based on Arduino Pro Mini, an MTK3339 GPS and an OLED display.
 */


#define GPS Serial
// https://github.com/PaulStoffregen/Time
#include <TimeLib.h>
// https://github.com/JChristensen/Timezone
#include <Timezone.h>
// https://github.com/adafruit/Adafruit_SSD1306
#include <Adafruit_SSD1306.h>

TimeChangeRule CEST={"CEST", Last, Sun, Mar, 2, 120};
TimeChangeRule CET={"CET", Last, Sun, Oct, 2, 60};
Timezone localTimezone(CEST,CET);

Adafruit_SSD1306 display(128, 64);

#define NMEA_BUFFER_LENGTH 15
#define NMEA_MAX_FIELDS 8
char nmeaFields[NMEA_MAX_FIELDS][NMEA_BUFFER_LENGTH]={};
uint8_t nmeaBufferPos=0;
uint8_t nmeaFieldNum=0;
uint32_t start;

void setup() {
	GPS.begin(9600);
	displayInit();
	display.clearDisplay();
	display.setTextSize(3);
	display.println(F("Waiting"));
	display.println(F("for GPS"));
	display.display();
	gpsInit();
}

void loop() {
	while(Serial.available()) {
		if(gpsProcess(Serial.read())) {
			/*
			for(uint8_t i=0; i<=4; i++) {
				Serial.print(i);
				Serial.print(": ");
				Serial.print(nmeaFields[i]);
				Serial.print(" (");
				Serial.print(strlen(nmeaFields[i]));
				Serial.println(')');
			}
			Serial.println();
			*/
			if(strcmp("GPZDA",nmeaFields[0])==0) {
				setTimeUTC();
				displayTime();
			}
			// This program is only interested in GPZDA. If GPRMC comes in,
			// which is the default setting in the GPS, we ask it to switch.
			// This works with MTK3339 and probably several others, even though
			// GPZDA is not even mentioned in the datasheet
			else if(strcmp("GPRMC",nmeaFields[0])==0) {
				Serial.println(F("Got unexpected GPRMC. Re-initializing GPS."));
				gpsInit();
			}
		}
	}
}

bool gpsProcess(const char c) {
	switch(c) {
		case '$':
			start=millis();
			for(uint8_t field=0; field<NMEA_MAX_FIELDS; field++)
				nmeaFields[field][0]='\0';
			nmeaFieldNum=0;
			nmeaBufferPos=0;
			break;
		case '*':
		case ',':
			nmeaFields[nmeaFieldNum][nmeaBufferPos]='\0';
			nmeaFieldNum++;
			nmeaBufferPos=0;
			break;
		case '\n':
			// TODO: Check parity
			return(true);
		default:
			// Intentionally one less than NMEA_BUFFER_LENGTH to make room for a null terminator
			if(nmeaBufferPos<NMEA_BUFFER_LENGTH)
				nmeaFields[nmeaFieldNum][nmeaBufferPos++]=c;
			break;
	}
	return(false);
}

void setTimeUTC() {
	//Serial.println(F("setTimeUTC called"));
	char buf[3]={};
	buf[2]='\0';
	buf[0]=nmeaFields[1][0];
	buf[1]=nmeaFields[1][1];
	uint8_t hours=atoi(buf);
	buf[0]=nmeaFields[1][2];
	buf[1]=nmeaFields[1][3];
	uint8_t minutes=atoi(buf);
	buf[0]=nmeaFields[1][4];
	buf[1]=nmeaFields[1][5];
	uint8_t seconds=atoi(buf);
	buf[0]=nmeaFields[2][0];
	buf[1]=nmeaFields[2][1];

	setTime(hours, minutes, seconds, atoi(nmeaFields[2]), atoi(nmeaFields[3]), atoi(nmeaFields[4]));
	adjustTime(0-(millis()-start)/1000);
}

void displayInit() {
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	while(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
		Serial.println(F("Display init failed"));
		delay(1000);
		Serial.println(F("Retrying..."));
	}
	display.setFont(0);
	display.setTextColor(WHITE);
}

void displayTime() {
	TimeChangeRule *tcr;
	time_t localTime=localTimezone.toLocal(now(), &tcr);
	display.clearDisplay();

	display.setTextSize(3);
	char minHoursBuf[]="00:00";
	int16_t minHoursX, minHoursY;
	uint16_t minHoursWidth,minHoursHeight;
	sprintf(minHoursBuf,"%02d:%02d", hour(localTime), minute(localTime));
	display.getTextBounds(minHoursBuf, 0, 0, &minHoursX, &minHoursY, &minHoursWidth, &minHoursHeight);
	display.setCursor(0,0);
	display.print(minHoursBuf);

	display.setTextSize(2);
	char secondsBuf[]="00:00";
	int16_t secondsX, secondsY;
	uint16_t secondsWidth,secondsHeight;
	sprintf(secondsBuf,":%02d", second(localTime));
	display.getTextBounds(secondsBuf, minHoursWidth, 0, &secondsX, &secondsY, &secondsWidth, &secondsHeight);
	display.setCursor(minHoursWidth,0);
	display.print(secondsBuf);

	display.setTextSize(1);
	int16_t tcrX, tcrY;
	uint16_t tcrWidth,tcrHeight;
	display.getTextBounds(tcr->abbrev, minHoursWidth, secondsHeight, &tcrX, &tcrY, &tcrWidth, &tcrHeight);
	display.setCursor(display.width()-tcrWidth, secondsHeight);
	display.print(tcr->abbrev);

	display.setTextSize(2);
	char dateBuf[]="XXXX-XX-XX";
	int16_t dateX, dateY;
	uint16_t dateWidth,dateHeight;
	sprintf(dateBuf,"%04d-%02d-%02d", year(localTime), month(localTime), day(localTime));
	// tcrHeight isn't really involved with the location, just used as a convenient height of text size 1 which works well as a separator
	display.getTextBounds(dateBuf, 0, minHoursHeight+tcrHeight, &dateX, &dateY, &dateWidth, &dateHeight);
	display.setCursor((display.width()-dateWidth)/2, minHoursHeight+tcrHeight);
	display.println(dateBuf);

	display.setTextSize(1);
	display.println();
	display.print(F("UTC: "));

	sprintf(dateBuf,"%04d-%02d-%02d", year(), month(), day());
	display.print(dateBuf);
	display.print(' ');

	sprintf(minHoursBuf,"%02d:%02d", hour(), minute());
	display.print(minHoursBuf);

	display.display();
}

void gpsInit() {
	// Enable only GPZDA (includes time and date, loaded from RTC without requiring a fix after warm boot)
	GPS.print(F("$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0*29\n"));
	// Enable only GPGGA and GPRMC
	//GPS.print(F("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\n"));
	// Enable only GPGGA (does not include date)
	//GPS.print(F("$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\n"));
	// Enable only GPRMC
	//GPS.print(F("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\n"));
}
