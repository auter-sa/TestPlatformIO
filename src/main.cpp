
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <FS.h>
#include <SPIFFS.h>
#include <DateTime.h>
#include "constants.h"
#include "utils.h"
#include "hexfile.h"
#include "schedule.h"

// Action buffer
int ledValue[8]    = { 0, 0, 0, 0, 0, 0, 0, 0 };
int oldLedValue[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

TaskHandle_t xHandleScheduleTask;
JsonDocument json;
time_t       offset;
Schedule    *schedule;

//-----------------------------------------------------------------------------

void writeLEDvalue() {
    uint8_t l1 = ((ledValue[0] & 0x01) > 0) || ((ledValue[0] & 0x08) > 0) || ((ledValue[0] & 0x40) > 0) ? LOW : HIGH;  // bits 1, 4, 7
    uint8_t l2 = ((ledValue[0] & 0x02) > 0) || ((ledValue[0] & 0x10) > 0) || ((ledValue[0] & 0x80) > 0) ? LOW : HIGH;  // bits 2, 5, 8
    uint8_t l3 = ((ledValue[0] & 0x04) > 0) || ((ledValue[0] & 0x20) > 0) ? LOW : HIGH;                                // bits 3, 6

    digitalWrite(LED1, l1);
    digitalWrite(LED2, l2);
    digitalWrite(LED3, l3);
}

//-----------------------------------------------------------------------------

void scheduleTask(void *pars) {
    bool changed = false;
    int  i;

    while (true) {
        for (i = 0; i < 8; i ++) {
            if (ledValue[i] != oldLedValue[i]) {
                changed = true;
                break;
            }
        }
        if (changed) {
            writeLEDvalue();
            for (i = 0; i < 8; i ++) {
                oldLedValue[i] = ledValue[i];
            }
        }
        vTaskDelay(Milliseconds2Ticks(100));
    }
}

//-----------------------------------------------------------------------------

String getTimeString(bool isHoliday) {
    const char dayNames[8] = "DLMWJVS";
    time_t     unixTime = DateTime.now() + offset;
    struct tm *timeInfo = localtime(&unixTime);
    char       timeString[24];

    sprintf(timeString, "%c%c %02d-%02d-%04d %02d:%02d:%02d", dayNames[timeInfo->tm_wday], (isHoliday ? 'f' : 'n'), timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900, timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
    return String(timeString);
}

//-----------------------------------------------------------------------------

void syncTime() {
    Serial.println("[1] Conexion a internet:");
    Serial.println();
    Serial.printf("    Conectandose al Wi-Fi: ");
    WiFi.begin("Visitas", "Meet2020C1.");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\r\n    Conectado!");
    Serial.println("");
    Serial.println("[2] Sincronizando fecha/hora con servidor:");

    HTTPClient http;
    offset = 0;
    while (offset == 0) {
        http.begin("http://worldtimeapi.org/api/timezone/America/Santiago");
        int httpCode = http.GET();
        if (httpCode > 0) {
            try {
                String jsonString = http.getString();

                if ((jsonString != NULL) && (! jsonString.isEmpty())) {
                    JsonDocument doc;
                    deserializeJson(doc, jsonString);

                    offset = doc["unixtime"].as<time_t>() + doc["raw_offset"].as<time_t>() + (doc["dst"].as<boolean>() ? doc["dst_offset"].as<time_t>() : 0) - DateTime.now();

                    Serial.println();
                    Serial.println("    Fecha y hora actuales segun servidor: " + doc["datetime"].as<String>());
                    Serial.println(doc["dst"].as<boolean>() ? "    Corresponde a horario de verano." : "    Corresponde a horario de invierno.");
                } else {  
                    Serial.println("    No se pudo obtener informacion del servidor. Reintentando...");
                }
            } catch (...) {
                Serial.println("    Error en el JSON recibidio del servidor. Reintentando...");
            }
        } else {
            Serial.println("    Error al obtener informacion del servidor: " + http.errorToString(httpCode) + ". Reintentando...");
        }
        http.end();
    }
    Serial.println();
}

void checkSchedule() {
    time_t     unixTime = DateTime.now() + offset;
    struct tm *timeInfo = localtime(&unixTime);

    if (timeInfo->tm_sec == 0) {
        Serial.println(getTimeString(schedule->isHoliday(timeInfo)));

        schedule->triggerActions(unixTime);

        if ((timeInfo->tm_min == 0) && (timeInfo->tm_hour == 0)) {  // Resync time at midnight
            Serial.println();
            syncTime();
        }
    }
}

//-----------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);

    HexFile *hf = new HexFile();
    schedule = hf->read(HEX_FILENAME);
    delete hf;
    schedule->display();

    syncTime();

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);

    time_t     unixTime = DateTime.now() + offset;
    struct tm *timeInfo = localtime(&unixTime);

    schedule->initActions(unixTime);
    xTaskCreatePinnedToCore(scheduleTask, "Schedule", 1024, NULL, 1, &xHandleScheduleTask, 0);
}

void loop() {
    static unsigned long lastCheck = 0;

    if (millis() - lastCheck >= 1000) {  // Checks each second
        lastCheck = millis();
        checkSchedule();
    }
}

//-----------------------------------------------------------------------------
