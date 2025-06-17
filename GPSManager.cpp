#include "GPSManager.h"
#include "LockGuard.h"
#include <TinyGPS++.h>

TinyGPSPlus gps;

GPSManager::GPSManager()
    : latitude(0.0), longitude(0.0), altitude(0.0), hdop(0.0),
      speed(0.0), course(0.0), utcTime(0), utcDate(0),
      satellites(0), fixValid(false),
      lastLatitude(0.0), lastLongitude(0.0),
      lastUtcTime(0), lastUtcDate(0), lastPositionValid(false),
      updateCallback(nullptr), mutex(xSemaphoreCreateMutex()),
      serialPort(nullptr), gpsStream(nullptr) {}

GPSManager::~GPSManager() {
    if (mutex) vSemaphoreDelete(mutex);
    closePreferences();
}

GPSManager& GPSManager::getInstance() {
    static GPSManager instance;
    return instance;
}

void GPSManager::begin(int rxPin, int txPin, int uartNum, uint32_t baudRate) {
    if (uartNum >= 0 && uartNum <= 2) {
        serialPort = new HardwareSerial(uartNum);
        serialPort->begin(baudRate, SERIAL_8N1, rxPin, txPin);
        gpsStream = serialPort;
    } else {
        gpsStream = &Serial;
    }

    initPreferences();
    loadLastPosition();

    xTaskCreatePinnedToCore(taskFunction, "GPS Task", 4096, this, 5, nullptr, 0);
}

void GPSManager::onDataUpdate(GPSDataUpdateCallback cb) {
    LockGuard lock(mutex);
    updateCallback = cb;
}

double GPSManager::getLatitude() const {
    LockGuard lock(mutex);
    return fixValid ? latitude : 0.0;
}

double GPSManager::getLongitude() const {
    LockGuard lock(mutex);
    return fixValid ? longitude : 0.0;
}

uint32_t GPSManager::getTimeUTC() const {
    LockGuard lock(mutex);
    return utcTime;
}

uint32_t GPSManager::getDate() const {
    LockGuard lock(mutex);
    return utcDate;
}

bool GPSManager::hasFix() const {
    LockGuard lock(mutex);
    return fixValid;
}

double GPSManager::getAltitude() const {
    LockGuard lock(mutex);
    return altitude;
}

double GPSManager::getSpeedKMPH() const {
    LockGuard lock(mutex);
    return speed;
}

double GPSManager::getCourse() const {
    LockGuard lock(mutex);
    return course;
}

uint8_t GPSManager::getSatsUsed() const {
    LockGuard lock(mutex);
    return satellites;
}

double GPSManager::getHDOP() const {
    LockGuard lock(mutex);
    return hdop;
}

double GPSManager::getLastLatitude() const {
    LockGuard lock(mutex);
    return lastPositionValid ? lastLatitude : 0.0;
}

double GPSManager::getLastLongitude() const {
    LockGuard lock(mutex);
    return lastPositionValid ? lastLongitude : 0.0;
}

uint32_t GPSManager::getLastTimeUTC() const {
    LockGuard lock(mutex);
    return lastUtcTime;
}

uint32_t GPSManager::getLastDate() const {
    LockGuard lock(mutex);
    return lastUtcDate;
}

bool GPSManager::hasLastValidPosition() const {
    LockGuard lock(mutex);
    return lastPositionValid;
}

void GPSManager::initPreferences() {
    preferences.begin(STORAGE_NAMESPACE, false);
}

void GPSManager::closePreferences() {
    preferences.end();
}

void GPSManager::saveLastPosition() {
    LockGuard lock(mutex);
    if (fixValid) {
        preferences.putDouble("lat", latitude);
        preferences.putDouble("lon", longitude);
        preferences.putUInt("time", utcTime);
        preferences.putUInt("date", utcDate);
        preferences.putBool("valid", true);

        lastLatitude = latitude;
        lastLongitude = longitude;
        lastUtcTime = utcTime;
        lastUtcDate = utcDate;
        lastPositionValid = true;
    } else {
        preferences.putBool("valid", false);
        lastPositionValid = false;
    }
}

void GPSManager::loadLastPosition() {
    LockGuard lock(mutex);
    bool valid = preferences.getBool("valid", false);
    if (valid) {
        lastLatitude = preferences.getDouble("lat", 0.0);
        lastLongitude = preferences.getDouble("lon", 0.0);
        lastUtcTime = preferences.getUInt("time", 0);
        lastUtcDate = preferences.getUInt("date", 0);
        lastPositionValid = true;
    } else {
        lastPositionValid = false;
    }
}

void GPSManager::taskFunction(void* pvParameters) {
    GPSManager* manager = static_cast<GPSManager*>(pvParameters);

    while (true) {
        while (manager->gpsStream->available()) {
            char c = manager->gpsStream->read();
            if (gps.encode(c)) {
                manager->processNMEA();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void GPSManager::processNMEA() {
    LockGuard lock(mutex);

    if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        fixValid = true;
    } else {
        fixValid = false;
    }

    if (gps.altitude.isValid()) {
        altitude = gps.altitude.meters();
    }

    if (gps.hdop.isValid()) {
        hdop = gps.hdop.value();
    }

    if (gps.speed.isValid()) {
        speed = gps.speed.kmph();
    }

    if (gps.course.isValid()) {
        course = gps.course.deg();
    }

    if (gps.satellites.isValid()) {
        satellites = gps.satellites.value();
    }

    if (gps.time.isValid()) {
        utcTime = gps.time.value();
    }

    if (gps.date.isValid()) {
        utcDate = gps.date.value();
    }

#ifdef GPS_DEBUG
    Serial.print("Lat: "); Serial.println(latitude, 6);
    Serial.print("Lon: "); Serial.println(longitude, 6);
    Serial.print("HDOP: "); Serial.println(hdop, 1);
    Serial.print("Satellites: "); Serial.println(satellites);
    Serial.println("-----------------------------");
#endif

    triggerCallbackIfChanged();
}

void GPSManager::triggerCallbackIfChanged() {
    if (updateCallback && fixValid) {
        updateCallback(latitude, longitude, hdop, satellites);
    }
}