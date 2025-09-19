
//Monitor Globals
const int BUFFER_SIZE = 15;
uint8_t buf[BUFFER_SIZE];
uint8_t char_buf[1];
bool awaitingData=false;
const byte START_BYTE=0xA5;
byte byteCount=0;

float voltage;
float current;
float soc;

//CAN Imports and Inits
#include <mcp_can.h>
#include <SPI.h>

MCP_CAN CAN0(17);     // Set CS to pin 17 for CanBed

//Data Global
byte data[4] = {0,0,0,0};

void setup() {
  //Serial Setup
  Serial.begin(115200);
  Serial1.begin(9600);

  //Can Setup
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");

  CAN0.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted
}

void updateMonitor(){  
  if(Serial1.available()){
      memset(char_buf,0,1);
      Serial1.readBytes(char_buf,1);

    if(!awaitingData){
      if(char_buf[0] == START_BYTE){
        awaitingData=true;
        byteCount=0;
        Serial.println("Now awaiting data.");
      }
    }else{
      //Serial.println("Incoming data");
        buf[byteCount] = char_buf[0];
        byteCount++;
        if(byteCount==15){
          for(int i=0;i<15;i++){
            Serial.print(buf[i],HEX);
            Serial.print("|");
          }Serial.println();
          uint16_t rawVoltage = ((uint16_t)buf[1] << 8) | buf[2];
          voltage = (float)rawVoltage / 100;

          int32_t rawCurrent= ((int32_t)buf[7] << 24) | (int32_t)(buf[8] << 16) | (int32_t)(buf[9] << 8) | (int32_t)(buf[10]);
          current = (float)rawCurrent/1000;
      
          uint8_t rawSoc=buf[0];
          soc=(float)rawSoc;

          Serial.print("Voltage: ");
          Serial.print(voltage);
          Serial.print(" | SOC: ");
          //Serial.print(buf[1]);
          Serial.print(soc);
          Serial.print(" | Current: ");
          Serial.print(current);
          Serial.print(" | Buffer length: ");
          Serial.print(sizeof(buf));
          Serial.print(" | Elapsed Time: ");
          Serial.println(millis()/1000);
          memset(buf,0,sizeof(buf));
          awaitingData=false;
        }
      }
    }
}

void updateData(){
  data[0]=voltage;
  data[1]=soc;
  data[2]=current;
  data[3]=(int)millis()/1000;
}

void sendCan(){
  // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  byte sndStat = CAN0.sendMsgBuf(0x100, 0, 4, data);
  if(sndStat == CAN_OK){
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Error Sending Message...");
  }
}

void loop() {
  updateMonitor();
  //Serial.println(voltage);
  updateData();
  sendCan();
  delay(1000);
}