
// This #include statement was automatically added by the Particle IDE.
#include "SparkFunLSM9DS1.h"

// This #include statement was automatically added by the Particle IDE.
#include "Kalman.h"

#include "math.h"

unsigned long lastRead= micros();
char myIpAddress[24];


//////////////////////////
// LSM9DS1 Library Init //
//////////////////////////
// Use the LSM9DS1 class to create an object. [imu] can be
// named anything, we'll refer to that throught the sketch.
LSM9DS1 imu;

///////////////////////g
// Example I2C Setup //
///////////////////////
// SDO_XM and SDO_G are both pulled high, so our addresses are:
#define LSM9DS1_M   0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // Would be 0x6A if SDO_AG is LOW

////////////////////////////
// Sketch Output Settings //
////////////////////////////
#define PRINT_CALCULATED
//#define PRINT_RAW
#define PRINT_SPEED 10 // 250 ms between prints

// Earth's magnetic field varies by location. Add or subtract
// a declination to get a more accurate heading. Calculate
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
//#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.

#define DECLINATION -13.57 // Zip Code for fairmeadow elementary school: 94306

int led=D7;
int i = 0;
// An UDP instance to let us send and receive packets over UDP
UDP Udp;

uint8_t server[] = {10,21,34,175}; // ip address of my phone
char gkx[64],gky[64],ghd[64],gax[64],gay[64],gaz[64];
IPAddress IPfromBytes( server );
unsigned int localPort = 33333;

uint32_t timer;

void setup()
{
    pinMode(led,OUTPUT);
  Serial.begin(115200);

   Particle.variable("ipAddress", myIpAddress, STRING);
    IPAddress myIp = WiFi.localIP();
    sprintf(myIpAddress, "%d.%d.%d.%d", myIp[0], myIp[1], myIp[2], myIp[3]);
     Serial.println(myIp[0]+myIp[1]+myIp[2]+myIp[3]);
   Particle.variable("kx",gkx,STRING);
   Particle.variable("ky",gky,STRING);
   Particle.variable("hd",ghd,STRING);
   Particle.variable("ax",gax,STRING);
   Particle.variable("ay",gay,STRING);
   Particle.variable("az",gaz,STRING);

  // start the UDP
  Udp.begin(localPort);

  // Before initializing the IMU, there are a few settings
  // we may need to adjust. Use the settings struct to set
  // the device's communication mode and addresses:
  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;
  // The above lines will only take effect AFTER calling
  // imu.begin(), which verifies communication with the IMU
  // and turns it on.
  if (!imu.begin())
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                  "work for an out of the box LSM9DS1 " \
                  "Breakout, but may need to be modified " \
                  "if the board jumpers are.");
    while (1)
      ;
  }

  timer = micros();

    lastRead = micros();
}

void loop()
{

    imu.readGyro();
    imu.readAccel();
    imu.readMag();
    printAttitude(imu.ax, imu.ay, imu.az, -imu.my, -imu.mx, imu.mz, imu.gx, imu.gy, imu.gz);

    //printGyro();

    delay(PRINT_SPEED);

}


void printGyro()
{
  // To read from the gyroscope, you must first call the
  // readGyro() function. When this exits, it'll update the
  // gx, gy, and gz variables with the most current data.
  imu.readGyro();


  // Now we can use the gx, gy, and gz variables as we please.
  // Either print them as raw ADC values, or calculated in DPS.
  Serial.print("G: ");
#ifdef PRINT_CALCULATED
  // If you want to print calculated values, you can use the
  // calcGyro helper function to convert a raw ADC value to
  // DPS. Give the function the value that you want to convert.
  Serial.print(imu.calcGyro(imu.gx), 2);
  Serial.print(", ");
  Serial.print(imu.calcGyro(imu.gy), 2);
  Serial.print(", ");
  Serial.print(imu.calcGyro(imu.gz), 2);
  Serial.println(" deg/s");
#elif defined PRINT_RAW
  Serial.print(imu.gx);
  Serial.print(", ");
  Serial.print(imu.gy);
  Serial.print(", ");
  Serial.println(imu.gz);
#endif

 char  ReplyBuffer[] = "acknowledged";
    Udp.beginPacket(IPfromBytes, localPort);
     sprintf(ReplyBuffer, "%f,%f,%f", imu.calcGyro(imu.gx),imu.calcGyro(imu.gy), imu.calcGyro(imu.gz));
   Udp.write(ReplyBuffer);

   Udp.endPacket();
}

void printAccel()
{
  // To read from the accelerometer, you must first call the
  // readAccel() function. When this exits, it'll update the
  // ax, ay, and az variables with the most current data.
  imu.readAccel();

  // Now we can use the ax, ay, and az variables as we please.
  // Either print them as raw ADC values, or calculated in g's.
  Serial.print("A: ");
#ifdef PRINT_CALCULATED
  // If you want to print calculated values, you can use the
  // calcAccel helper function to convert a raw ADC value to
  // g's. Give the function the value that you want to convert.
  Serial.print(imu.calcAccel(imu.ax), 2);
  Serial.print(", ");
  Serial.print(imu.calcAccel(imu.ay), 2);
  Serial.print(", ");
  Serial.print(imu.calcAccel(imu.az), 2);
  Serial.println(" g");
#elif defined PRINT_RAW
  Serial.print(imu.ax);
  Serial.print(", ");
  Serial.print(imu.ay);
  Serial.print(", ");
  Serial.println(imu.az);
#endif

}


Kalman kalmanX; // Create the Kalman instances
Kalman kalmanY;
double gyroXangle, gyroYangle; // Angle calculate using the gyro only
double compAngleX, compAngleY; // Calculated angle using a complementary filter
double kalAngleX, kalAngleY; // Calculated angle using a Kalman filter

// Calculate pitch, roll, and heading.
// Pitch/roll calculations take from this app note:
// http://cache.freescale.com/files/sensors/doc/app_note/AN3461.pdf?fpsp=1
// Heading calculations taken from this app note:
// http://www51.honeywell.com/aero/common/documents/myaerospacecatalog-documents/Defense_Brochures-documents/Magnetic__Literature_Application_notes-documents/AN203_Compass_Heading_Using_Magnetometers.pdf
void printAttitude(
float ax, float ay, float az, float mx, float my, float mz, float gx, float gy, float gz)
{
  float roll = atan2(ay, az);
  float pitch = atan2(-ax, sqrt(ay * ay + az * az));

  float heading;
  if (my == 0)
    heading = (mx < 0) ? 180.0 : 0;
  else
    heading = atan2(mx, my);

  heading -= DECLINATION * M_PI / 180;

  if (heading > M_PI) heading -= (2 * M_PI);
  else if (heading < -M_PI) heading += (2 * M_PI);
  else if (heading < 0) heading += 2 * M_PI;


  double dt = (double)(micros() - timer) / 1000000; // Calculate delta time
  timer = micros();

  // Convert everything from radians to degrees:
  heading *= 180.0 / M_PI;
  pitch *= 180.0 / M_PI;
  roll  *= 180.0 / M_PI;

  double gyroXrate = gx / 131.0; // Convert to deg/s
  double gyroYrate = gy / 131.0; // Convert to deg/s


  if ((pitch < -90 && kalAngleY > 90) || (pitch > 90 && kalAngleY < -90)) {
    kalmanY.setAngle(pitch);
    compAngleY = pitch;
    kalAngleY = pitch;
    gyroYangle = pitch;
  } else
    kalAngleY = kalmanY.getAngle(pitch, gyroYrate, dt); // Calculate the angle using a Kalman filter

  if (abs(kalAngleY) > 90)
    gyroXrate = -gyroXrate; // Invert rate, so it fits the restriced accelerometer reading
  kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculate the angle using a Kalman filter

double accelX = imu.calcAccel(ax);
double accelY = imu.calcAccel(ay);
double accelZ = imu.calcAccel(az);
    digitalWrite(led,HIGH);
      char kx[32],ky[32],hd[32],zax[32],zay[32],zaz[32];
      sprintf(kx,"%.2f",kalAngleX);
      sprintf(ky,"%.2f",kalAngleY);
      sprintf(hd,"%.2f",heading);
      sprintf(zax,"%.2f",accelX);
      sprintf(zay,"%.2f",accelY);
      sprintf(zaz,"%.2f",accelZ);
      sprintf(gkx, "%s", kx);
      sprintf(gky, "%s", ky);
      sprintf(ghd, "%s", hd);
      sprintf(gax, "%s", zax);
      sprintf(gay, "%s", zay);
      sprintf(gaz, "%s", zaz);
      delay(3000);
      digitalWrite(led,LOW);
     delay(3000);

  char  ReplyBuffer[] = "acknowledged";
    Udp.beginPacket(IPfromBytes, localPort);
     sprintf(ReplyBuffer, "%.2f,%.2f,%.2f,A,%.2f,%.2f,%.2f", kalAngleX,kalAngleY, heading, accelX,accelY,accelZ);
     //sprintf(ReplyBuffer, "%.2f,%.2f,%.2f", kalAngleX,kalAngleY, heading);

   Udp.write(ReplyBuffer);

   Udp.endPacket();

  Serial.println(ReplyBuffer);

}
