
//Battery Monitor Globals
const int BUFFER_SIZE = 2;
uint8_t buf[BUFFER_SIZE];
uint8_t char_buf[1];
bool awaitingData = false;
const byte START_BYTE = 0xA5;
byte byteCount = 0;

//Data globals
float voltage;
float current;
float soc;
byte data[4] = { 0, 0, 0, 0 };

//CAN Imports and Inits
#include <mcp_can.h>  //library for interacting with CAN (for RaceCapture)
#include <SPI.h>      //dependency for mcp_can

//initialize CAN
MCP_CAN CAN0(17);  // Set CS to pin 17 for CanBed

void setup() {

  //Serial Setup
  Serial.begin(115200);  //Debug serial
  Serial1.begin(9600);   //Battery monitor serial

  //CAN Setup
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
    Serial.println("MCP2515 Initialized Successfully!");
  } else {
    Serial.println("Error Initializing MCP2515...");
  }

  CAN0.setMode(MCP_NORMAL);  // Change to normal mode to allow messages to be transmitted
}


//Helper function for updating battery monitor
//the logic is cursed: do not use as a template
void updateMonitor() {
  if (Serial1.available()) {
    memset(char_buf, 0, 1);
    Serial1.readBytes(char_buf, 1);

    if (!awaitingData) {
      if (char_buf[0] == START_BYTE) {
        awaitingData = true;
        byteCount = 0;
        Serial.println("Now awaiting data.");
      }
    } else {
      //Serial.println("Incoming data");
      buf[byteCount] = char_buf[0];
      byteCount++;
      if (byteCount == 2) {
        for (int i = 0; i < 2; i++) {
          Serial.print(buf[i], HEX);
          Serial.print("|");
        }
        Serial.println();

        uint8_t rawSoc = buf[0];
        soc = (float)rawSoc;

        Serial.print(" | SOC: ");
        //Serial.print(buf[1]);
        Serial.print(soc);
        Serial.print(" | Buffer length: ");
        Serial.print(sizeof(buf));
        Serial.print(" | Elapsed Time: ");
        Serial.println(millis() / 1000);
        memset(buf, 0, sizeof(buf));
        awaitingData = false;
      }
    }
  }
}


//Helper function for preping data to send
void updateData() {
  data[0] = 0;
  data[1] = soc;
  data[2] = 0;
  data[3] = (int)millis() / 1000;
}


//helper function for sending data
void sendCan() {
  // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  byte sndStat = CAN0.sendMsgBuf(0x100, 0, 4, data);
  if (sndStat == CAN_OK) {
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Error Sending Message...");
  }
}

//Main loop
void loop() {
  updateMonitor(); //get battery data
  updateData(); //prep data
  sendCan(); //send data
}