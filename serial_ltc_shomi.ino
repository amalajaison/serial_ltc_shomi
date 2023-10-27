/*
 The circuit:

  * SDI - to digital pin 11 (MOSI pin)
  * SDO - to digital pin 12 (MISO pin)
  * CLK - to digital pin 13 (SCK pin)
*/
#include "SPI.h" // necessary library
int del=3; // used for various delays

int dac1=10; // CS lines for each SPI device
int dac2=9;
int dac3=8;
int dac4=7;


// LTC SPI communications parameters
uint8_t chipSelect;
uint8_t channel;

// Parameters related to parsing serial input
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

// variables to hold the parsed data
char messageFromPC[numChars] = {0};
int channelFromPC = 0;
int chipSelectFromPC = 0;
float floatFromPC = 0.0;

boolean newData = false;


void setup()
{
  Serial.begin(9600);
  Serial.println("Enter data in this style <SET chipSelect channel voltage>");   

// when the Arduino first powers on, it should not immediately start sending commands to the DAC's
// because they might take time to power on.
// So, we insert a sleep here to make sure the DAC's (and everything else) have time to power on.
  delay(1000);
  Serial.println("Sleep command completed.");


  pinMode(dac1, OUTPUT);
  pinMode(dac2, OUTPUT);
  pinMode(dac3, OUTPUT);
  pinMode(dac4, OUTPUT);
  digitalWrite(dac1, HIGH);
  digitalWrite(dac2, HIGH);
  digitalWrite(dac3, HIGH);
  digitalWrite(dac4, HIGH);
  /*
   * Set the pins’ state to HIGH. 
   * Otherwise more than one CS pins may be initially low in some instances 
   * and cause the first data sent from MOSI to travel along to two or more SPI devices
   */
  SPI.begin(); // wake up the SPI bus.
  // Changing the output range to +/- 10 Volts
  digitalWrite(dac1, LOW);
  //LTC2668_write(LTC2668_CS,LTC2668_CMD_WRITE_N_UPDATE_N, selected_dac, dac_code);
  SPI.transfer16(0x00e0); //Set span for all DACs
  SPI.transfer16(0x0003);
  /*
  LTC2668_SPAN_0_TO_5V             0x0000
  LTC2668_SPAN_0_TO_10V            0x0001
  LTC2668_SPAN_PLUS_MINUS_5V       0x0002
  LTC2668_SPAN_PLUS_MINUS_10V      0x0003
  LTC2668_SPAN_PLUS_MINUS_2V5      0x0004
   */  
  digitalWrite(dac1, HIGH);
  
  digitalWrite(dac2, LOW);
  SPI.transfer16(0x00e0); //Set span for all DACs
  SPI.transfer16(0x0003);
  digitalWrite(dac2, HIGH);

  digitalWrite(dac3, LOW);
  SPI.transfer16(0x00e0); //Set span for all DACs
  SPI.transfer16(0x0003);
  digitalWrite(dac3, HIGH);

  digitalWrite(dac4, LOW);
  SPI.transfer16(0x00e0); //Set span for all DACs
  SPI.transfer16(0x0003);
  digitalWrite(dac4, HIGH);

  //SPI.setBitOrder(MSBFIRST);
  
}


void loop()
{
    recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            //   because strtok() used in parseData() replaces the commas with \0
        parseData();
        showParsedData();


        if(!strncmp(messageFromPC, "SET", 3)) {
          // set voltage and enable
          Serial.println("Setting");

          chipSelect=chipSelectFromPC;
          channel=channelFromPC;

          digitalWrite(chipSelect, LOW);
          
          SPI.transfer16(0x0030|channel);
          SPI.transfer16(dac_value(floatFromPC));
          
          digitalWrite(chipSelect, HIGH);
          
        } else if(!strncmp(messageFromPC, "MUX", 3)) {
          
          Serial.println("Changing MUX");
          chipSelect=chipSelectFromPC;
          channel=channelFromPC;
          
          digitalWrite(chipSelect, LOW);
 
          SPI.transfer16(0x00b0);
          SPI.transfer16(0x0010|channel);
          /*
          LTC2668_MUX_DISABLE              0x0000  //! Disable MUX
          LTC2668_MUX_ENABLE               0x0010  //! Enable MUX, OR with MUX channel to be monitored
          */
          digitalWrite(chipSelect, HIGH);

        }

        newData = false;
    }
}

//============

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

//============

void parseData() {      // split the data into its parts

    char * strtokIndx; // this is used by strtok() as an index
    char delimiter[4]=", ";

    strtokIndx = strtok(tempChars,delimiter);      // get the first part - the string
    strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC

    strtokIndx = strtok(NULL,delimiter); // this continues where the previous call left off
    chipSelectFromPC = atoi(strtokIndx);     // convert this part to an integer
 
    strtokIndx = strtok(NULL,delimiter); // this continues where the previous call left off
    channelFromPC = atoi(strtokIndx);     // convert this part to an integer

    strtokIndx = strtok(NULL,delimiter);
    floatFromPC = atof(strtokIndx);     // convert this part to a float

}

//============

void showParsedData() {
    Serial.print("Message ");
    Serial.println(messageFromPC);
    Serial.print("CS_Integer ");
    Serial.println(chipSelectFromPC);
    Serial.print("Integer ");
    Serial.println(channelFromPC);
    Serial.print("Float ");
    Serial.println(floatFromPC);
}

uint16_t dac_value(float volts) {
  float minv=-10.0;
  float maxv=10.0;
  return uint16_t((volts-minv)/(maxv-minv)*65535);  
}
