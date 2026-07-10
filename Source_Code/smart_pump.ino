#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Smart Irrigation"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

#define SOIL_SENSOR_PIN 34
#define RELAY_PIN 25

int moistureLevel = 0;
int timerSeconds = 0;
unsigned long pumpOffTime = 0;
bool manualRunning = false;

BlynkTimer timer;

// Timer value from Blynk
BLYNK_WRITE(V2)
{
    timerSeconds = param.asInt();
}

// Manual Pump Control
BLYNK_WRITE(V0)
{
    int btn = param.asInt();

    if (btn == 1)
    {
        digitalWrite(RELAY_PIN, HIGH);
        manualRunning = true;

        if (timerSeconds > 0)
        {
            pumpOffTime = millis() + (timerSeconds * 1000);
        }
        else
        {
            pumpOffTime = 0;
        }
    }
    else
    {
        digitalWrite(RELAY_PIN, LOW);
        manualRunning = false;
        pumpOffTime = 0;
    }
}

// Automatic Soil Moisture Monitoring
void checkSoil()
{
    moistureLevel = analogRead(SOIL_SENSOR_PIN);

    int percentage = map(moistureLevel, 0, 4095, 100, 0);

    Blynk.virtualWrite(V1, percentage);

    if (!manualRunning)
    {
        if (percentage < 30)
        {
            digitalWrite(RELAY_PIN, HIGH);
        }
        else
        {
            digitalWrite(RELAY_PIN, LOW);
        }
    }
}

// Automatic Pump Timer
void checkPumpTimer()
{
    if (manualRunning && pumpOffTime > 0)
    {
        if (millis() >= pumpOffTime)
        {
            digitalWrite(RELAY_PIN, LOW);
            manualRunning = false;
            pumpOffTime = 0;

            Blynk.virtualWrite(V0, 0);
        }
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    timer.setInterval(3000L, checkSoil);
    timer.setInterval(500L, checkPumpTimer);
}

void loop()
{
    Blynk.run();
    timer.run();
}