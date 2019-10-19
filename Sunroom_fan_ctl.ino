#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <EEPROM.h>

/* This driver uses the Adafruit unified sensor library (Adafruit_Sensor),
   which provides a common 'type' for sensor data and some helper functions.
   
   To use this driver you will also need to download the Adafruit_Sensor
   library and include it in your libraries folder.

   You should also assign a unique ID to this sensor for use with
   the Adafruit Sensor API so that you can identify this particular
   sensor in any data logs, etc.  To assign a unique ID, simply
   provide an appropriate value in the constructor below (12345
   is used by default in this example).
   
   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3V DC or VIN on BMP180 board to 5V DC 
   Connect GROUND to common ground
    
   History
   =======
   2013/JUN/17  - Updated altitude calculations (KTOWN)
   2013/FEB/13  - First version (KTOWN)
*/

int Btn_start_temp = 2;
int Btn_stop_temp = 3;
int ledPin = 7;
int val;
int fan_status = 0;
float tempSRoomF, start_temp, stop_temp;
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);


/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void displaySensorDetails(void)
{
  sensor_t sensor;
  bmp.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

/**************************************************************************/
//    SETUP    SETUP     SETUP     SETUP     SETUP    SETUP
/**************************************************************************/
void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Pressure Sensor Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  displaySensorDetails();
  Serial.println("Sunroom_fan_ctl");
  pinMode(Btn_start_temp, INPUT);
  pinMode(Btn_stop_temp, INPUT);
  pinMode(ledPin, OUTPUT);

  EEPROM.get(0, start_temp);
  EEPROM.get(5, stop_temp);
  Serial.print("Initial start_temp - ");
  Serial.print(start_temp);
  Serial.print(" ---- stop_temp - ");   
  Serial.println(stop_temp); 
}

/**************************************************************************/
//    LOOP     LOOP    LOOP     LOOP     LOOP     LOOP     LOOP
/**************************************************************************/
void loop(void) 
{
  /* Get a new sensor event */ 
  sensors_event_t event;
  bmp.getEvent(&event);
 
  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  {
    /* Display atmospheric pressue in hPa */
    //Serial.print("Pressure:    ");
    //Serial.print(event.pressure);
    //Serial.println(" hPa");
    
    /* Calculating altitude with reasonable accuracy requires pressure    *
     * sea level pressure for your position at the moment the data is     *
     * converted, as well as the ambient temperature in degress           *
     * celcius.  If you don't have these values, a 'generic' value of     *
     * 1013.25 hPa can be used (defined as SENSORS_PRESSURE_SEALEVELHPA   *
     * in sensors.h), but this isn't ideal and will give variable         *
     * results from one day to the next.                                  *
     *                                                                    *
     * You can usually find the current SLP value by looking at weather   *
     * websites or from environmental information centers near any major  *
     * airport.                                                           *
     *                                                                    *
     * For example, for Paris, France you can check the current mean      *
     * pressure and sea level at: http://bit.ly/16Au8ol                   */
     
    /* First we get the current temperature from the BMP085 */
    float temperature;
    bmp.getTemperature(&temperature);
    Serial.print("Temperature: ");
    //Serial.print(temperature);
    //Serial.println(" C");
    tempSRoomF = (temperature * 9.0 / 5.0) + 32.0;
    Serial.print(tempSRoomF);
    Serial.print(" F ");

    /* Then convert the atmospheric pressure, SLP and temp to altitude    */
    /* Update this next line with the current SLP for better results      */
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
    //Serial.print("Altitude:    "); 
    //Serial.print(bmp.pressureToAltitude(seaLevelPressure,
                                     //   event.pressure,
                                     //   temperature)); 
    //Serial.println(" m");
    //Serial.println("");
  }
  else
  {
    Serial.println("Sensor error");
  }

  val = digitalRead(Btn_start_temp);  //Serial.print(" green =  "); Serial.print(val);
  if (val == LOW)
  {
    start_temp = tempSRoomF;
    EEPROM.put(0, start_temp);
  }
  
  val =  digitalRead(Btn_stop_temp);  val = digitalRead(Btn_stop_temp);  //Serial.print(" red =  "); Serial.println(val);
  if (val == LOW)
  {
    stop_temp = tempSRoomF;
    EEPROM.put(5, stop_temp);
  }
  
  if (start_temp < tempSRoomF)
  {
    fan_status = 1;
    digitalWrite(ledPin, HIGH);
    delay(30000);
  }
    if (stop_temp > tempSRoomF)
  {
    fan_status = 0;
    digitalWrite(ledPin, LOW);
    delay(30000);
  }

  Serial.print(" Start Fan at "); Serial.print(start_temp);
  Serial.print(" Stop Fan at "); Serial.print(stop_temp);
  Serial.print(" Fan is "); Serial.println(fan_status);
  delay(1000);
}
