// when the water is low, how often should the alarm sound, 5 minutes is 300 seconds
#define LOW_WATER_WARNING_SECONDS_BETWEEN 10

// when water is low, how many beeps
#define LOW_WATER_WARNING_BEEP_COUNT 4

// duration of each buzzer tone in milliseconds
#define BUZZER_TONE_DURATION 100

// frequency of tone for water low
#define BUZZER_TONE_WATER_LOW_FREQUENCY 3000

// frequency for each tone when water is low
#define BUZZER_TONE_WATER_LOW_INCREMENT 750


// water pump relay module active for seconds
#define WATER_PUMP_ACTIVE_SECONDS 30

// water pump relay module inactive for seconds
#define WATER_PUMP_INACTIVE_SECONDS 240

// low water average rating
#define WATER_SENSOR_MINIMUM 510


// short delay for tone
#define BEEP_SHORT_DELAY 130



// Do not touch below this line

// queue
#define INT_MIN 0

struct QItem
{
	int data;
	QItem* next;
	
	QItem(int d)
	{
		data = d;
		next = NULL;
	}
};

class Queue {
private:
    int _capacity, _size;
    QItem *_front, *_rear;
public:
    Queue(int capacity);
    bool isFull();
    bool isEmpty();
    void enqueue(int item);
    int dequeue();
    int average();
};


// Hydroponic specific
#define MILLIS_PER_SECOND 1000
#define BUZZER_PIN 2
#define WATER_PUMP_RELAY_MODULE_PIN 3

long sleepMillis = MILLIS_PER_SECOND - (LOW_WATER_WARNING_BEEP_COUNT * BEEP_SHORT_DELAY);


// water sensor
#define MAX_SENSOR_ENTRIES 15
#define WATER_SENSOR_PIN A0
#define WATER_SENSOR_READING_SECONDS_BETWEEN 1
unsigned long lastWaterLowBuzzer = 0;
bool waterLow = true;
Queue sensorQueue(MAX_SENSOR_ENTRIES);
unsigned long waterSensorLastChecked = 0;

// water pump
bool waterPumpActive = false;
unsigned long waterPumpLastAction = 0;

void setup()
{
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(WATER_PUMP_RELAY_MODULE_PIN, OUTPUT);
}

void loop()
{
    unsigned long currMillis = millis();
    bool waterToneSounded = soundWaterLow(currMillis);

    updateWaterPumpActiveState(currMillis);
    updateWaterLevel(currMillis);
    delay(waterToneSounded ? sleepMillis : MILLIS_PER_SECOND);
}

void updateWaterPumpActiveState(unsigned long currMillis)
{
    if (waterPumpActive)
    {
        Serial.print("Water pump curr millis: ");
        Serial.print(currMillis);
        Serial.print("; next deactivate: ");
        Serial.println(waterPumpLastAction + (WATER_PUMP_ACTIVE_SECONDS * MILLIS_PER_SECOND));

        if (currMillis > waterPumpLastAction + (WATER_PUMP_ACTIVE_SECONDS * MILLIS_PER_SECOND))
        {
            digitalWrite(WATER_PUMP_RELAY_MODULE_PIN, LOW);
            waterPumpActive = false;
            waterPumpLastAction = currMillis;
        }
    }
    else if (!waterLow && currMillis > waterPumpLastAction + (WATER_PUMP_INACTIVE_SECONDS * MILLIS_PER_SECOND))
    {
        digitalWrite(WATER_PUMP_RELAY_MODULE_PIN, HIGH);
        waterPumpActive = true;
        waterPumpLastAction = currMillis;
    }
}

void updateWaterLevel(unsigned long currMillis)
{
    if (waterSensorLastChecked + (WATER_SENSOR_READING_SECONDS_BETWEEN * MILLIS_PER_SECOND) < currMillis)
    {
        int sensorValue = analogRead(WATER_SENSOR_PIN);

        if (sensorQueue.isFull())
          sensorQueue.dequeue();
          
        sensorQueue.enqueue(sensorValue);
        int sensorAverage = sensorQueue.average();
        waterSensorLastChecked = currMillis;

        if (sensorQueue.isFull())
        {
            waterLow = sensorAverage < WATER_SENSOR_MINIMUM;
        }
        
        Serial.print("Water Sensor Average: "); 
        Serial.print(sensorAverage);
        Serial.print("; Water Sensor Value: "); 
        Serial.println(sensorValue); 
    }
}

void doToneSound(int delayMillis, int frequency, int duration)
{
    tone(BUZZER_PIN, frequency, duration);
    delay(delayMillis);
}

bool soundWaterLow(unsigned long currMillis)
{
    if (!waterLow)
        return false;

    bool canPlay = (lastWaterLowBuzzer + (LOW_WATER_WARNING_SECONDS_BETWEEN * MILLIS_PER_SECOND)) < currMillis;

    if (canPlay)
    {
        lastWaterLowBuzzer = currMillis;
        for (int i = 0; i < LOW_WATER_WARNING_BEEP_COUNT; i++)
        {
            doToneSound(BEEP_SHORT_DELAY, BUZZER_TONE_WATER_LOW_FREQUENCY + (i * BUZZER_TONE_WATER_LOW_INCREMENT), BUZZER_TONE_DURATION);
        }

        return true;
    }

    return false;
}

// queue code 
Queue::Queue(int capacity)
{
	_front = NULL;
	_rear = NULL;
    _capacity = capacity;
	_size = 0;
}
 
// Queue is full when size
// becomes equal to the capacity
bool Queue::isFull()
{
    return (_size == _capacity);
}
 
// Queue is empty when size is 0
bool Queue::isEmpty()
{
    return (_size == 0);
}

void Queue::enqueue(int item)
{
    if (isFull())
        return;
	
   _size++;
	QItem* temp = new QItem(item);
	
	if (_rear == NULL)
	{
		_front = _rear = temp;
		return;
	}

    _rear->next = temp;
	_rear = temp;
}
 
int Queue::dequeue()
{
    if (isEmpty())
        return INT_MIN;
        
    QItem* temp = _front;
	_front = _front->next;
    _size--;
	
	if (_front == NULL)
		_rear = NULL;
	
	int value = temp->data;
	delete(temp);
	
    return value;
}
 
int Queue::average()
{
    if (isEmpty() || _front == NULL)
	{
        return INT_MIN;
	}
        
    int sum = 0;
	QItem* temp = _front;
	
	while (temp != NULL)
	{
		sum += temp->data;
		temp = temp->next;
	}
	
    return (sum / _size);
}