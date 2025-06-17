#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Preferences.h>

// #define GPS_DEBUG

class GPSManager {
public:
    using GPSDataUpdateCallback = void (*)(double lat, double lon, double hdop, uint8_t sats);

    static GPSManager& getInstance();

    void begin(int rxPin, int txPin, int uartNum = 1, uint32_t baudRate = 115200);
    void onDataUpdate(GPSDataUpdateCallback cb);

    double getLatitude() const;
    double getLongitude() const;
    uint32_t getTimeUTC() const;
    uint32_t getDate() const;
    bool hasFix() const;

    double getAltitude() const;
    double getSpeedKMPH() const;
    double getCourse() const;
    uint8_t getSatsUsed() const;
    double getHDOP() const;


    double getLastLatitude() const;
    double getLastLongitude() const;
    uint32_t getLastTimeUTC() const;
    uint32_t getLastDate() const;
    bool hasLastValidPosition() const;

    void saveLastPosition();
    void loadLastPosition();

private:
    GPSManager();
    ~GPSManager();

    static void taskFunction(void* pvParameters);
    void processNMEA();
    void triggerCallbackIfChanged();

    void initPreferences();
    void closePreferences();

    double latitude;
    double longitude;
    double altitude;
    double hdop;
    double speed;
    double course;
    uint32_t utcTime;
    uint32_t utcDate;
    uint8_t satellites;
    bool fixValid;

    double lastLatitude;
    double lastLongitude;
    uint32_t lastUtcTime;
    uint32_t lastUtcDate;
    bool lastPositionValid;

    GPSDataUpdateCallback updateCallback;

    SemaphoreHandle_t mutex;
    HardwareSerial* serialPort;
    Stream* gpsStream;

    Preferences preferences;
    static constexpr const char* STORAGE_NAMESPACE = "gps_data";
};

#endif // GPS_MANAGER_H