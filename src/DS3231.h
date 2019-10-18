#include <Wire.h>
#define DS3231_I2C_ADDRESS 0x68

byte decToBcd(byte val)
{
    return ((val / 10 * 16) + (val % 10));
}

byte bcdToDec(byte val)
{
    return ((val / 16 * 10) + (val % 16));
}

timeStruct readTime()
{
    timeStruct time;
    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    Wire.write(0); // set DS3231 register pointer to 00h
    Wire.endTransmission();
    Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
    // request seven bytes of data from DS3231 starting from register 00h
    time.seconds = (int)bcdToDec(Wire.read() & 0x7f); //sekundy
    time.minutes = (int)bcdToDec(Wire.read());        //minuty
    time.hours = (int)bcdToDec(Wire.read() & 0x3f); //hodiny
    return time;
}

uint8_t setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
    // sets time and date data to DS3231
    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    Wire.write(0);                    // set next input to start at the seconds register
    Wire.write(decToBcd(second));     // set seconds
    Wire.write(decToBcd(minute));     // set minutes
    Wire.write(decToBcd(hour));       // set hours
    Wire.write(decToBcd(dayOfWeek));  // set day of week (1=Sunday, 7=Saturday)
    Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
    Wire.write(decToBcd(month));      // set month
    Wire.write(decToBcd(year));       // set year (0 to 99)
    return Wire.endTransmission();
}

uint8_t setDS3231time(struct timeStruct time)
{
    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    Wire.write(0);                    // set next input to start at the seconds register
    Wire.write(decToBcd(time.seconds));     // set seconds
    Wire.write(decToBcd(time.minutes));     // set minutes
    Wire.write(decToBcd(time.hours));       // set hours
    Wire.write(decToBcd(1));  // set day of week (1=Sunday, 7=Saturday)
    Wire.write(decToBcd(1)); // set date (1 to 31)
    Wire.write(decToBcd(1));      // set month
    Wire.write(decToBcd(1));       // set year (0 to 99)
    return Wire.endTransmission();
}