#include <Arduino.h>
#include <Wire.h>

#include "Adafruit_MLX90614.h"
#include <DigitalTube.h>
#include <string.h>

const short buzzer_pin = 2;
const short wait_LED_pin = 8;   // Yellow LED
const short pass_LED_pin = 9;   // Green LED
const short covid_LED_pin = 10; // Red LED
const short infrared_pin = 13;  // Infrared sensor

const short pump_pin = 12; // Pump's relay
const short clock_pin = 5; // ╗
const short latch_pin = 4; // ║═>> 3 pin นี้ ต่อเข้ากับ 7 segments (จอตัวเลขอะ)
const short data_pin = 3;  // ╝

DigitalTube dis = DigitalTube(clock_pin, latch_pin, data_pin);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup()
{
    // Set modes for pins

    pinMode(buzzer_pin, OUTPUT);
    pinMode(wait_LED_pin, OUTPUT);
    pinMode(pass_LED_pin, OUTPUT);
    pinMode(covid_LED_pin, OUTPUT);
    pinMode(infrared_pin, INPUT);
    pinMode(pump_pin, OUTPUT);

    dis.begin(); // Initiates display sensor
    mlx.begin(); // Initiates temperature sensor
}

bool doneOnce = false; // ตัวแปรสำหรับดูว่าวัดไปรึยัง
void loop()
{
    bool isActivated = !digitalRead(infrared_pin);

    if (isActivated)
    {
        // Keep doing when activated
    }
    else
    {
        // Keep doing when not activated
        doneOnce = false;
        char *idle_text = "   Hands HErE    ";
        size_t size = strlen(idle_text);
        short n_round = (size - 4) < 0 ? 1 : (size - 4) + 1;
        for (int i = 0; i < n_round && digitalRead(infrared_pin); i++) // ให้ข้อความในตัวแปร idle_text วิ่งไปเรื่อยๆจนกว่าจะตรวจเจอมือ
        {
            dis.print(idle_text + i);
            char istr = i + '0';
            delay(300);
        }
    }

    if (isActivated && !doneOnce) // Do once per activation
    {
        delay(500); // รอให้มือเข้าไปถึงตัววัดอุณหภูมิจริงๆ

        digitalWrite(wait_LED_pin, HIGH); // Yellow LED on

        // วัดอุณหภูมิ 20 ครั้ง แล้วจับหารเฉลี่ย
        double temp[20];
        for (int round = 0; round < 20; round++)
        {
            char *dot_str[4] = {" .   ", "  .  ", "   . ", "    ."}; // ใส่ไว้เฉยๆ ของจริงๆไม่ใช่ข้อความชุดนี้แน่นอน เป็นข้อความที่ขึ้นระหว่างรอวัดอุณหภูมิเสร็จ
            for (int i = 0; i < 4; i++)
            {
                dis.print(dot_str[(round + 1) / 5]);
            }
            temp[round] = mlx.readObjectTempC();
            delay(100);
        }
        digitalWrite(wait_LED_pin, LOW); // Yellow LED off

        // Temperature handling
        double avg_temp = average(temp, 20);
        if (avg_temp >= 1037) // If temperature is not available
        {
            dis.print("NULL");
        }
        else // Handles read temperature
        {
            digitalBlink(pump_pin, 50);   // พ่นน้ำ
            digitalBlink(buzzer_pin, 30); // ส่งเสียง
            dis.print(avg_temp);
            if (avg_temp > 37.5)
            {
                digitalBlink(covid_LED_pin, 1000); // Red LED on
            }
            else
            {
                digitalBlink(pass_LED_pin, 1000); // Green LED on
            }
        }
        doneOnce = true;

        delay(500);
    }
    delay(10); // ใส่ไว้เฉยๆ กลัวทำงานหนักเกิน
}

void digitalBlink(short pin, unsigned long duration_ms) // Function ย่อๆ เอาไว้ส่ง HIGH รอเสี้ยววิแล้วส่ง LOW ไปที่ digital pin
{
    digitalWrite(pin, HIGH);
    delay(duration_ms);
    digitalWrite(pin, LOW);
}

double average(double *array, size_t size) // Function สำหรับเฉลี่ย
{
    double sum = 0;
    for (size_t i = 0; i < size; i++)
    {
        sum += array[i];
    }
    return sum / size;
}
