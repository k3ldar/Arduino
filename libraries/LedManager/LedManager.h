#ifndef __LedManager__H
#define __LedManager__H

#include <Arduino_LED_Matrix.h>
#include "Common.h"
#include "WebClientManager.h"

#define LED_UPDATE_FREQUENCY 500

const long rssiRate[5] = {-70, -80, -90, -100, -110 };
const int MaxLedRows = 8;
const int MaxLedColumns = 12;
const uint8_t LedOn = 1;
const uint8_t LedOff = 0;

#if defined(LED_WEATHER_STATION)
const int8_t DoNotMonitor = -128;
#endif
#if defined(LED_WATER_PUMP)
const int8_t PumpOneColumn = 5;
const int8_t PumpTwoColumn = 9;
#endif

#if defined(LED_WEATHER_STATION) && defined(LED_WATER_PUMP)
  Can not have both defined at same time
#endif

/*

  { X, X, X, X, X, X, X, X, X, X, X, X },
  { X, X, X, X, X, X, X, X, X, X, X, X },
  { X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { X, X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { X, X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { X, X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
  
*/
class LedManager
{
private:
	unsigned long _nextLedUpdate = 0;
	ArduinoLEDMatrix _matrix;
	uint8_t _ledFrame[MaxLedRows][MaxLedColumns];

#if defined(LED_WEATHER_STATION)
	uint8_t _rainingState = 0;
	uint64_t _lastRainUpdate = 0;
	float _temperature = DoNotMonitor;
	uint64_t _lastTemperatureUpdate = 0;
	float _humididty = DoNotMonitor;
	uint64_t _lastHumidityUpdate = 0;
	void updateRainingState(uint64_t currMillis);
	void updateTemperature(uint64_t currMillis);
	void updateHumidity(uint64_t currMillis);
#endif

#if defined(LED_WATER_PUMP)
	uint64_t _lastPumpOneUpdate = 0;
	uint64_t _lastPumpTwoUpdate = 0;
	byte _pumpOneStatus = 0;
	byte _pumpTwoStatus = 0;
	bool _pumpOneRunning = false;
	bool _pumpTwoRunning = false;
	void updatePumpOne(uint64_t currMillis);
	void updatePumpTwo(uint64_t currMillis);
#endif

	void updateLed(int delayMs);
public:
	LedManager();
	void Initialize();
	void ProcessLedMatrix(WebClientManager *webClient, uint64_t currMillis);
	void UpdateSignalStrength(int16_t strenth);
	void UpdateWebCommunication(bool isSending);
	void UpdateWifiFailure(int8_t failures);
	void UpdateFailureCount(int8_t failures);
	void UpdateConnectedState(int32_t status);
	void UpdateLedFrame(uint8_t state);
	void UpdateRow(uint8_t row, uint8_t state);
	void UpdateColumn(uint8_t column, uint8_t state);
	void StartupSequence();
	void ShutdownSequence();
	
#if defined(LED_WEATHER_STATION)
	void SetIsRaining(bool isRaining);
	void SetTemperature(float temperature);
	void SetHumidity(float humidity);
#endif

#if defined(LED_WATER_PUMP)
	void SetPumpOne(bool isRunning);
	void SetPumpTwo(bool isRunning);
#endif
};


#endif
