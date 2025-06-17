#include <TinyGPS++.h>
#include "GPSManager.h"

void onGPSUpdate(double lat, double lon, double hdop, uint8_t sats) {
    Serial.print("Lat: "); Serial.println(lat, 6);
    Serial.print("Lon: "); Serial.println(lon, 6);
    Serial.print("HDOP: "); Serial.println(hdop, 1);
    Serial.print("Sats: "); Serial.println(sats);
    
    GPSManager::getInstance().saveLastPosition();
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    GPSManager& gps = GPSManager::getInstance();
    gps.begin(16, 17, 2, 9600);
    gps.onDataUpdate(onGPSUpdate);

    if (gps.hasLastValidPosition()) {
        Serial.println("Последняя сохранённая позиция:");
        Serial.print("Lat: ");
        Serial.println(gps.getLastLatitude(), 6);
        Serial.print("Lon: ");
        Serial.println(gps.getLastLongitude(), 6);
        Serial.print("Time: ");
        Serial.println(gps.getLastTimeUTC());
        Serial.print("Date: ");
        Serial.println(gps.getLastDate());
    } else {
        Serial.println("Нет сохранённой позиции");
    }
}

void loop() {
    delay(100);
}