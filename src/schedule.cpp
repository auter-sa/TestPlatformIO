#include <Arduino.h>
#include "constants.h"
#include "schedule.h"

//-----------------------------------------------------------------------------

ScheduleHoliday::ScheduleHoliday() {
    this->day   = 0;
    this->month = 0;
    this->year  = 0;
}

ScheduleHoliday::ScheduleHoliday(int day, int month) {
    this->day   = day;
    this->month = month;
    this->year  = 0;
}

ScheduleHoliday::ScheduleHoliday(int day, int month, int year) {
    this->day   = day;
    this->month = month;
    this->year  = 2000 + year;
}

ScheduleHoliday::~ScheduleHoliday() {
}

void ScheduleHoliday::setDay(int day) {
    this->day = day;
}

void ScheduleHoliday::setMonth(int month) {
    this->month = month;
}

void ScheduleHoliday::setYear(int year) {
    this->year = 2000 + year;
}

int ScheduleHoliday::getDay() {
    return this->day;
}

int ScheduleHoliday::getMonth() {
    return this->month;
}

int ScheduleHoliday::getYear() {
    return this->year;
}

//-----------------------------------------------------------------------------

ScheduleEntry::ScheduleEntry() {
    this->hour     = "00:00";
    this->days     = 0;
    this->action   = ' ';
    this->argument = 0;
    this->seasons  = 0;
}

ScheduleEntry::ScheduleEntry(String hour, int days, char action) {
    this->setAll(hour, days, action);
}

ScheduleEntry::ScheduleEntry(String hour, int days, char action, int argument, int seasons) {
    this->setAll(hour, days, action, argument, seasons);
}

ScheduleEntry::~ScheduleEntry() {
}

void ScheduleEntry::setAll(String hour, int days, char action) {
    this->hour     = hour;
    this->days     = days;
    this->action   = action;
    this->argument = 0;
    this->seasons  = 0;
}

void ScheduleEntry::setAll(String hour, int days, char action, int argument, int seasons) {
    this->hour     = hour;
    this->days     = days;
    this->action   = action;
    this->argument = argument;
    this->seasons  = seasons;
}

String ScheduleEntry::getHour() const {
    return this->hour;
}

int ScheduleEntry::getDays() const {
    return this->days;
}

char ScheduleEntry::getAction() const {
    return this->action;
}

int ScheduleEntry::getArgument() const {
    return this->argument;
}

int ScheduleEntry::getSeasons() const {
    return this->seasons;
}

String ScheduleEntry::bitsToString(int numValue, String strValues) {
    char mask[9];
    int i, bit = 0x01;

    for (i = 0; i < 8; i ++) {
        mask[i] = ((numValue & bit) > 0) ? strValues.charAt(i) : '.';
        bit <<= 1;
    }
    mask[i] = '\0';
    return String(mask);
}

String ScheduleEntry::get() {
    char buffer[80];
    sprintf(buffer, "%s - Dias: %s - Acc: %c - Arg: %s - Est: %s",
            this->hour, bitsToString(this->days, "LMWJVSDF"), this->action, bitsToString(this->argument, "12345678"),  bitsToString(this->seasons, "12345678"));
    return ((String) buffer);
}

//-----------------------------------------------------------------------------

ScheduleSeason::ScheduleSeason() {
    this->day    = 0;
    this->month  = 0;
    this->season = 0;
}

ScheduleSeason::ScheduleSeason(int day, int month, int season) {
    this->day    = day;
    this->month  = month;
    this->season = season;
}

ScheduleSeason::~ScheduleSeason() {
}

void ScheduleSeason::setDay(int day) {
    this->day = day;
}

void ScheduleSeason::setMonth(int month) {
    this->month = month;
}

void ScheduleSeason::setSeason(int year) {
    this->season = season;
}

int ScheduleSeason::getDay() {
    return this->day;
}

int ScheduleSeason::getMonth() {
    return this->month;
}

int ScheduleSeason::getSeason() {
    return this->season;
}

//-----------------------------------------------------------------------------

Schedule::Schedule(byte *hexFile) {
    int  i, index;
    byte b1, b2, b3, b4, b5, b6, b7;
    byte hwYear = 0;
    bool oldFormat = false;

	// Fixed holidays
    for (i = 0, index = 5; i < MAX_FIXED_HOLIDAYS; i ++) {
		b1 = hexFile[index ++];
		b2 = hexFile[index ++];
		if ((b1 != (byte) 0x00) && (b2 != (byte) 0x00)) {
            ScheduleHoliday *holiday = new ScheduleHoliday(b1, b2);
            this->holidays.push_back(holiday);
		}
	}

    // Seasons
    for (i = 0; i < MAX_SEASONS; i ++) {
		b1 = hexFile[index ++];
		b2 = hexFile[index ++];
		b3 = hexFile[index ++];
		if ((b1 != (byte) 0x00) && (b2 != (byte) 0x00) && (b3 != (byte) 0x00)) {
            if (i < MAX_SEASONS - 1) {
                ScheduleSeason *season = new ScheduleSeason(b1, b2, b3);
                this->seasons.push_back(season);
            } else {
                if (b3 > (byte) 0x08) {
                    oldFormat = true;
                    index -= 3;
                } else {
                    ScheduleSeason *season = new ScheduleSeason(b1, b2, b3);
                    this->seasons.push_back(season);
                }
            }
        }
    }

    //index    += (MAX_SEASONS - 1) * 3;
    //oldFormat = ((hexFile[index] != (byte) 0x00) && (hexFile[index + 1] != (byte) 0x00) && (hexFile[index] > (byte) 0x08));
    //if (! oldFormat) {
    //    index += 3;
    //}

	for (i = 0; i < MAX_VARIABLE_HOLIDAYS - (oldFormat ? 2 : 0); i ++) {
		b1 = hexFile[index ++];
		b2 = hexFile[index ++];
		b3 = hexFile[index ++];
		if ((b1 != (byte) 0x00) && (b2 != (byte) 0x00) && (b3 != (byte) 0x00)) {
            ScheduleHoliday *holiday = new ScheduleHoliday(b1, b2, b3);
            this->holidays.push_back(holiday);
            if (hwYear == 0) {
                hwYear = b3;
            }
		}
	}

    // Schedule
    for (i = 0; i < MAX_SCHEDULE_ENTRIES - (oldFormat ? 1 : 0);  i ++) {
		b1 = hexFile[index ++];
		b2 = hexFile[index ++];
		b3 = hexFile[index ++];
		b4 = hexFile[index ++];
		b5 = hexFile[index ++];
		b6 = hexFile[index ++];
		b7 = hexFile[index ++];
        if ((b1 != (byte) 0xFF) && (b2 != (byte) 0xFF) && (b3 != (byte) 0xFF)) {
            ScheduleEntry *entry = new ScheduleEntry(hourString(b1, b2), b4, (char) b5, b6, b7);
            this->entries.push_back(entry);
        }
    }
    std::sort(this->entries.begin(), this->entries.end(), [this] (const ScheduleEntry *a, const ScheduleEntry *b) { return this->compareEntry(a, b); } );

	// Holy week 
	for (i = 0; i < MAX_HOLY_WEEK; i ++) {
		b1 = hexFile[index ++];
		b2 = hexFile[index ++];
		if ((b1 != (byte) 0x00) && (b2 != (byte) 0x00)) {
            ScheduleHoliday *holiday = new ScheduleHoliday(b1, b2, hwYear ++);
            this->holidays.push_back(holiday);
		}
	}
}

Schedule::~Schedule() {
    for (ScheduleEntry *entry : this->entries) {
        if (entry != NULL) {
            delete entry;
        }
    }
    this->entries.clear();

    for (ScheduleHoliday *holiday : this->holidays) {
        if (holiday != NULL) {
            delete holiday;
        }
    }
    this->holidays.clear();

    for (ScheduleSeason *season : this->seasons) {
        if (season != NULL) {
            delete season;
        }
    }
    this->seasons.clear();
}

String Schedule::hourString(int hour, int minute) {
    char hString[6];
    sprintf(hString, "%02d:%02d", hour, minute);
    return(String(hString));
}

String Schedule::hourString(int hour, int minute, int second) {
    char hString[9];
    sprintf(hString, "%02d:%02d:%02", hour, minute, second);
    return(String(hString));
}

void Schedule::display() {
    Serial.println();
    Serial.println("[0] Contenidos de archivo HEX");
    Serial.println();
    Serial.println("Feriados:");
    if (this->holidays.size() > 0) {
        Serial.println();
        for (ScheduleHoliday *holiday : this->holidays) {
            Serial.printf("    %02d-%02d-%04d\r\n", holiday->getDay(), holiday->getMonth(), holiday->getYear());
        }
    }
    Serial.println();
    Serial.println("Estaciones:");
    if (this->seasons.size() > 0) {
        Serial.println();
        for (ScheduleSeason *season : this->seasons) {
            Serial.printf("    %02d-%02d - %d\r\n", season->getDay(), season->getMonth(), season->getSeason());
        }
    }
    Serial.println();
    Serial.println("Tabla horaria:");
    if (this->entries.size() > 0) {
        Serial.println();
        for (ScheduleEntry *entry : this->entries) {
            Serial.println("    " + entry->get());
        }
    }
    Serial.println();
}

bool Schedule::isHoliday(struct tm *timeInfo) {
    int day   = timeInfo->tm_mday;
    int month = timeInfo->tm_mon + 1;
    int year  = timeInfo->tm_year + 1900;

    for (ScheduleHoliday *holiday : holidays) {
        if ((holiday->getDay() == day) && (holiday->getMonth() == month) && ((holiday->getYear() == 0) || (holiday->getYear() == year))) {
            return true;
        }
    }
    return false;
}

int Schedule::currentSeason(struct tm *timeInfo) {
    int currSeason = 0;
    int day        = timeInfo->tm_mday;
    int month      = timeInfo->tm_mon + 1;

    for (ScheduleSeason *season : this->seasons) {
        if ((month > season->getMonth()) || ((month == season->getMonth()) && (day >= season->getDay()))) {
            currSeason = season->getSeason();
        }
    }
    return currSeason;
}


bool Schedule::compareEntry(const ScheduleEntry *entry1, const ScheduleEntry *entry2) {
    if (! entry1->getHour().equals(entry2->getHour())) {
        return entry1->getHour() < entry2->getHour();
    }
    if (entry1->getDays() != entry2->getDays()) {
        return entry1->getDays() < entry2->getDays();
    }
    return entry1->getAction() < entry2->getAction();
}

void Schedule::initActions(time_t unixTime) {
    struct tm *timeInfo = localtime(&unixTime);
    char       hour[10], lastHour[10] = "00:00";
    int        dow      = timeInfo->tm_wday - 1;
    int        season   = currentSeason(timeInfo);

    sprintf(hour, "%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min);
    if (dow < 0) {
        dow = 7;
    }
    dow = (1 << dow) | (isHoliday(timeInfo) ? 128 : 0);

    // Check previous hour while triggering all actions to get an initial value;
    for (ScheduleEntry *entry : this->entries) {
        triggerAction(entry);
        Serial.println(">>> [Ini] <" + String(season) + "> " + entry->get());
        if (entry->getHour() <= hour) {
            strcpy(lastHour, entry->getHour().c_str());
        }
    }
    for (ScheduleEntry *entry : this->entries) {
        if (((entry->getDays() & dow) > 0) && (entry->getHour() <= lastHour) && (((1 << (season - 1)) & entry->getSeasons()) > 0)) {
            triggerAction(entry);
            Serial.println(">>> [Ini] <" + String(season) + "> " + entry->get());
        }
    }
}

void Schedule::triggerActions(time_t unixTime) {
    struct tm *timeInfo = localtime(&unixTime);
    char       hour[10];
    int        dow      = timeInfo->tm_wday - 1;
    int        season   = currentSeason(timeInfo);

    if (timeInfo->tm_sec == 0) {
        sprintf(hour, "%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min);
        if (dow < 0) {
            dow = 7;
        }
        dow = (1 << dow) | (isHoliday(timeInfo) ? 128 : 0);

        for (ScheduleEntry *entry : this->entries) {
            if (((entry->getDays() & dow) > 0) && (entry->getHour().equals(hour)) && (((1 << (season - 1)) & entry->getSeasons()) > 0)) {
                triggerAction(entry);
                Serial.println(">>> <" + String(season) + "> " + entry->get());
            }
        }
    }
}

void Schedule::triggerAction(ScheduleEntry *action) {
    // This should be the function that does the actual task on the Arduino machine
    // ledValue is a global variable
    if (action->getAction() == 'E') {  // Turns on leds
        ledValue[0] |= (byte) action->getArgument();
    }
    if (action->getAction() != 'E') {  // Turns off leds
        ledValue[0] &= (! (byte) action->getArgument());
    }
}

//-----------------------------------------------------------------------------
