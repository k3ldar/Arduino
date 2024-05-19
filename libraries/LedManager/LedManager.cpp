#include "LedManager.h"

LedManager::LedManager()
{
	UpdateLedFrame(LedOff);
}

void LedManager::Initialize()
{
	_matrix.begin();
}

void LedManager::UpdateConnectedState(int32_t status)
{
	_ledFrame[6][1] = LedOff;
	_ledFrame[7][1] = LedOff;
	
    if (status == WL_CONNECTED)
    {
		_ledFrame[6][1] = LedOn;
		_ledFrame[7][1] = LedOn;
    }
    else if (status == WL_CONNECTING)
    {
		_ledFrame[7][1] = LedOn;
    }
}

void LedManager::UpdateFailureCount(int8_t failures)
{
    for (int i = 0; i < WIFI_FAILURES_FOR_RESTART; i++)
        _ledFrame[0][i] = failures > i ? LedOn : LedOff;
}

void LedManager::UpdateWifiFailure(int8_t failures)
{
    for (int i = 0; i < WIFI_FAILURES_FOR_RESTART; i++)
        _ledFrame[1][i] = failures > i ? LedOn : LedOff;
}

void LedManager::UpdateWebCommunication(bool isSending)
{
	_ledFrame[5][1] = isSending ? LedOn : LedOff;
}

void LedManager::UpdateSignalStrength(int16_t strenth)
{
    _ledFrame[MaxLedRows -1][0] = LedOn;

    for (int i = 2; i < MaxLedRows -1; i++)
        _ledFrame[i][0] = strenth >= rssiRate[i -2] ? LedOn : LedOff;

    if (strenth == 0)
    {
        _ledFrame[2][0] = LedOff;
        _ledFrame[4][0] = LedOff;
        _ledFrame[6][0] = LedOff;
    }
}

void LedManager::ProcessLedMatrix(WebClientManager *webClient, uint64_t currMillis)
{
	bool hasDelay = false;
	
	if (webClient)
	{
		if (webClient->getRequestSent() || webClient->postRequestSent())
		{
			UpdateWebCommunication(true);
		}
		else
		{
			UpdateWebCommunication(false);
			hasDelay = true;
		}

		int failures = webClient->socketConnectFailures();

		UpdateFailureCount(failures);
		UpdateWifiFailure(webClient->wifiConnectFailures());

		bool canUpdate = currMillis > _nextLedUpdate;
		
		if (canUpdate)
		{
			UpdateSignalStrength(webClient->getRssi());
			UpdateConnectedState(webClient->getWiFiStatus());
			_nextLedUpdate = currMillis + 300;
		}
	}
		
#if defined(LED_WEATHER_STATION)
	updateRainingState(currMillis);
	updateTemperature(currMillis);
	updateHumidity(currMillis);
#endif

#if defined(LED_WATER_PUMP)
	updatePumpOne(currMillis);
	updatePumpTwo(currMillis);
#endif

	updateLed(0);
    
	if (hasDelay)
        delay(50);
}

void LedManager::UpdateLedFrame(uint8_t state)
{
	for (int i = 0; i < MaxLedRows; i++)
	{
		for (int j = 0; j < MaxLedColumns; j++)
		{
			_ledFrame[i][j] = state;
		}
	}
}

void LedManager::UpdateRow(uint8_t row, uint8_t state)
{
	if (row > MaxLedRows -1)
		return;
	
	for (int i = 0; i < MaxLedColumns; i++)
	{
		_ledFrame[row][i] = state;
	}
}

void LedManager::UpdateColumn(uint8_t column, uint8_t state)
{
	if (column > MaxLedColumns -1)
		return;
	
	for (int i = 0; i < MaxLedRows; i++)
	{
		_ledFrame[i][column] = state;
	}
}
	
void LedManager::StartupSequence()
{
	for (int i = 0; i < MaxLedRows; i++)
	{
		if (i % 2)
		{
			for (int j = MaxLedColumns -1; j >= 0; j--)
			{
				_ledFrame[i][j] = LedOn;
				updateLed(30);
			}
		}
		else
		{
			for (int j = 0; j < MaxLedColumns; j++)
			{
				_ledFrame[i][j] = LedOn;
				updateLed(30);
			}
		}
	}
	
	bool isOn = true;
	
	for (int i = 0; i < 10; i++)
	{
		UpdateLedFrame(isOn ? LedOn : LedOff);
		isOn = !isOn;
		updateLed(200);
	}
}

void LedManager::ShutdownSequence()
{
	UpdateLedFrame(LedOff);
	updateLed(100);
	int topRow = 0;
	int bottomRow = 7;
	int leftColumn = 0;
	int rightColumn = 11;
	
	for (int i = 0; i < 5; i++)
	{
		UpdateRow(topRow, LedOn);
		UpdateRow(bottomRow, LedOn);
		UpdateColumn(leftColumn, LedOn);
		UpdateColumn(rightColumn, LedOn);
		updateLed(400);
		
		topRow++;
		bottomRow--;
		leftColumn++;
		rightColumn--;
	}
	
	UpdateLedFrame(LedOff);
	updateLed(500);
	
	_ledFrame[2][3] = LedOn;
	_ledFrame[2][4] = LedOn;
	_ledFrame[2][7] = LedOn;
	_ledFrame[2][8] = LedOn;
	updateLed(800);
}

#if defined(LED_WEATHER_STATION)

void LedManager::SetIsRaining(bool isRaining)
{
	_rainingState = isRaining ? 1 : 0;
}

void LedManager::SetTemperature(float temperature)
{
	_temperature = temperature;
}

void LedManager::SetHumidity(float humidity)
{
	_humididty = humidity;
}
#endif

#if defined(LED_WATER_PUMP)
void LedManager::SetPumpOne(bool isRunning)
{
	_pumpOneRunning = isRunning;
}

void LedManager::SetPumpTwo(bool isRunning)
{
	_pumpTwoRunning = isRunning;
}
#endif

// Private methods

void LedManager::updateLed(int delayMs)
{
	_matrix.renderBitmap(_ledFrame, MaxLedRows, MaxLedColumns);
	delay(delayMs);
}

#if defined(LED_WEATHER_STATION)
void LedManager::updateTemperature(uint64_t currMillis)
{
	if (currMillis < _lastTemperatureUpdate)
		return;
	
	_ledFrame[7][4] = _temperature > DoNotMonitor && _temperature < 0 ? LedOn : LedOff;
	_ledFrame[6][4] = _temperature > 0 ? LedOn : LedOff;
	_ledFrame[5][4] = _temperature > 8 ? LedOn : LedOff;
	_ledFrame[4][4] = _temperature > 16 ? LedOn : LedOff;
	_ledFrame[3][4] = _temperature > 24 ? LedOn : LedOff;
	_ledFrame[2][4] = _temperature > 32 ? LedOn : LedOff;
	
	_lastTemperatureUpdate = currMillis + LED_UPDATE_FREQUENCY;
}

void LedManager::updateHumidity(uint64_t currMillis)
{
	if (currMillis < _lastHumidityUpdate)
		return;
	
	_ledFrame[7][5] = _humididty > 40 ? LedOn : LedOff;
	_ledFrame[6][5] = _humididty > 50 ? LedOn : LedOff;
	_ledFrame[5][5] = _humididty > 60 ? LedOn : LedOff;
	_ledFrame[4][5] = _humididty > 70 ? LedOn : LedOff;
	_ledFrame[3][5] = _humididty > 80 ? LedOn : LedOff;
	_ledFrame[2][5] = _humididty > 90 ? LedOn : LedOff;
	
	_lastHumidityUpdate = currMillis + LED_UPDATE_FREQUENCY;
}

void LedManager::updateRainingState(uint64_t currMillis)
{
	if (currMillis < _lastRainUpdate)
		return;

	if (_rainingState > 0)
		_rainingState++;
	
	_ledFrame[3][7] = _rainingState > 1 && _rainingState < 3 ? LedOn : LedOff;
	_ledFrame[3][9] = _rainingState > 1 && _rainingState < 3 ? LedOn : LedOff;
	_ledFrame[4][8] = _rainingState > 2 && _rainingState < 4 ? LedOn : LedOff;
	_ledFrame[4][10] = _rainingState > 2 && _rainingState < 4 ? LedOn : LedOff;
	_ledFrame[5][7] = _rainingState > 3 && _rainingState < 5 ? LedOn : LedOff;
	_ledFrame[5][9] = _rainingState > 3 && _rainingState < 5 ? LedOn : LedOff;
	_ledFrame[6][8] = _rainingState > 4 && _rainingState < 6 ? LedOn : LedOff;
	_ledFrame[6][10] = _rainingState > 4 && _rainingState < 6 ? LedOn : LedOff;
	_ledFrame[7][7] = _rainingState > 5 ? LedOn : LedOff;
	_ledFrame[7][9] = _rainingState > 5 ? LedOn : LedOff;
	
	if (_rainingState == 0)
		return;
	
	if (_rainingState > 5)
		_rainingState = 1;
	
	_lastRainUpdate = currMillis + LED_UPDATE_FREQUENCY;
}

#endif

#if defined(LED_WATER_PUMP)

void LedManager::updatePumpOne(uint64_t currMillis)
{
	if (currMillis < _lastPumpOneUpdate)
		return;
	
	_pumpOneStatus++;
	
	if (_pumpOneStatus > 3)
		_pumpOneStatus = 0;


	_ledFrame[6][PumpOneColumn - 1] = _pumpOneRunning && _pumpOneStatus == 0 ? LedOn : LedOff;
	_ledFrame[6][PumpOneColumn + 1] = _pumpOneRunning && _pumpOneStatus == 0 ? LedOn : LedOff;
	
	_ledFrame[5][PumpOneColumn - 1] = _pumpOneRunning && _pumpOneStatus == 1 ? LedOn : LedOff;
	_ledFrame[5][PumpOneColumn + 1] = _pumpOneRunning && _pumpOneStatus == 1 ? LedOn : LedOff;
	
	_ledFrame[4][PumpOneColumn - 1] = _pumpOneRunning && _pumpOneStatus == 2 ? LedOn : LedOff;
	_ledFrame[4][PumpOneColumn + 1] = _pumpOneRunning && _pumpOneStatus == 2 ? LedOn : LedOff;
	
	_ledFrame[3][PumpOneColumn - 1] = _pumpOneRunning && _pumpOneStatus == 3 ? LedOn : LedOff;
	_ledFrame[3][PumpOneColumn] = _pumpOneRunning && _pumpOneStatus == 3 ? LedOn : LedOff;
	_ledFrame[3][PumpOneColumn + 1] = _pumpOneRunning && _pumpOneStatus == 3 ? LedOn : LedOff;
	
	_ledFrame[7][PumpOneColumn] = _pumpOneRunning && _pumpOneStatus == 0 ? LedOn : LedOff;
	_ledFrame[6][PumpOneColumn] = _pumpOneRunning && (_pumpOneStatus == 0 || _pumpOneStatus == 1) ? LedOn : LedOff;
	_ledFrame[5][PumpOneColumn] = _pumpOneRunning && (_pumpOneStatus == 1 || _pumpOneStatus == 2) ? LedOn : LedOff;
	_ledFrame[4][PumpOneColumn] = _pumpOneRunning && (_pumpOneStatus == 2 || _pumpOneStatus == 3) ? LedOn : LedOff;
	

	_lastPumpOneUpdate = currMillis + LED_UPDATE_FREQUENCY;
}

void LedManager::updatePumpTwo(uint64_t currMillis)
{
	if (currMillis < _lastPumpTwoUpdate)
		return;
	
	_pumpTwoStatus++;
	
	if (_pumpTwoStatus > 3)
		_pumpTwoStatus = 0;

	_ledFrame[6][PumpTwoColumn - 1] = _pumpOneRunning && _pumpTwoStatus == 0 ? LedOn : LedOff;
	_ledFrame[6][PumpTwoColumn + 1] = _pumpOneRunning && _pumpTwoStatus == 0 ? LedOn : LedOff;
	
	_ledFrame[5][PumpTwoColumn - 1] = _pumpOneRunning && _pumpTwoStatus == 1 ? LedOn : LedOff;
	_ledFrame[5][PumpTwoColumn + 1] = _pumpOneRunning && _pumpTwoStatus == 1 ? LedOn : LedOff;
	
	_ledFrame[4][PumpTwoColumn - 1] = _pumpOneRunning && _pumpTwoStatus == 2 ? LedOn : LedOff;
	_ledFrame[4][PumpTwoColumn + 1] = _pumpOneRunning && _pumpTwoStatus == 2 ? LedOn : LedOff;
	
	_ledFrame[3][PumpTwoColumn - 1] = _pumpOneRunning && _pumpTwoStatus == 3 ? LedOn : LedOff;
	_ledFrame[3][PumpTwoColumn] = _pumpOneRunning && _pumpTwoStatus == 3 ? LedOn : LedOff;
	_ledFrame[3][PumpTwoColumn + 1] = _pumpOneRunning && _pumpTwoStatus == 3 ? LedOn : LedOff;
	
	_ledFrame[7][PumpTwoColumn] = _pumpOneRunning && _pumpTwoStatus == 0 ? LedOn : LedOff;
	_ledFrame[6][PumpTwoColumn] = _pumpOneRunning && (_pumpTwoStatus == 0 || _pumpTwoStatus == 1) ? LedOn : LedOff;
	_ledFrame[5][PumpTwoColumn] = _pumpOneRunning && (_pumpTwoStatus == 1 || _pumpTwoStatus == 2) ? LedOn : LedOff;
	_ledFrame[4][PumpTwoColumn] = _pumpOneRunning && (_pumpTwoStatus == 2 || _pumpTwoStatus == 3) ? LedOn : LedOff;
	
	_lastPumpTwoUpdate = currMillis + LED_UPDATE_FREQUENCY;
}


#endif
