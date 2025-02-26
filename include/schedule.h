#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

extern int ledValue[8];  // Action buffer from main.cpp

class ScheduleHoliday {
    private:
        int day;
        int month;
        int year;

    public:
        ScheduleHoliday();
        ScheduleHoliday(int day, int month);
        ScheduleHoliday(int day, int month, int year);
        ~ScheduleHoliday();

        void setDay(int day);
        void setMonth(int month);
        void setYear(int year);
        int  getDay();
        int  getMonth();
        int  getYear();
};

class ScheduleEntry {
    private:
        String hour;
        int    days;
        char   action;
        int    argument;
        int    stations;

        String bitsToString(int numValue, String strValues);

    public:
        ScheduleEntry();
        ScheduleEntry(String hour, int days, char action);
        ScheduleEntry(String hour, int days, char action, int argument, int stations);
        ~ScheduleEntry();

        void   setAll(String hour, int days, char action);
        void   setAll(String hour, int days, char action, int argument, int stations);
        String getHour() const;
        int    getDays() const;
        char   getAction() const;
        int    getArgument() const;
        int    getStations() const;

        String get();
};

class Schedule {
    private:
        std::vector<ScheduleHoliday *> holidays;
        std::vector<ScheduleEntry *> entries;

        String hourString(int hour, int minute);
        String hourString(int hour, int minute, int second);
        bool   compareEntry(const ScheduleEntry *entry1, const ScheduleEntry *entry2);

    public:
        Schedule(JsonArray items);
        Schedule(byte *hexFile);
        ~Schedule();

        bool isHoliday(struct tm *timeInfo);
        void display();
        void initActions(time_t unixTime);
        void triggerActions(time_t unixTime);
        void triggerAction(ScheduleEntry *action);

        //std::vector<ScheduleEntry *> getAllActions();
        //int                          getNumActions();
        //ScheduleEntry               *getAction(int index);
        //std::vector<ScheduleEntry *> getActions(time_t unixTime);
        //std::vector<ScheduleEntry *> firstActions(time_t unixTime);
    };

#endif