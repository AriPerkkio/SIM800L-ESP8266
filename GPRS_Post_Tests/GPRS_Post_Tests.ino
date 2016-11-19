#include <SoftwareSerial.h>

#define DEBUG 1
#define SIM800_TX_PIN 8
#define SIM800_RX_PIN 7

SoftwareSerial serialSIM800(SIM800_TX_PIN,SIM800_RX_PIN);
char responseBuffer[256];

char *gprsStart[]     = {
                          (char *)"AT+SAPBR=3,1,\"Contype\", \"GPRS\"\r\n", // Configure bearer profile
                          (char *)"AT+SAPBR=3,1,\"APN\",\"internet\"\r\n",
                          (char *)"AT+SAPBR=1,1\r\n", // Open a GPRS context
                          (char *)"AT+SAPBR=2,1\r\n" // Query the GPRS context
                        };
char *gprsEnd[]       = { (char *)"AT+SAPBR=0,1\r\n" }; // Close a GPRS context

char *httpStart[]     = {
                          (char *)"AT+HTTPINIT\r\n", // Init HTTP service
                          (char *)"AT+HTTPPARA=\"CID\",1\r\n", // Set parameters for HTTP session
                          (char *)"AT+HTTPPARA=\"URL\",\"83.212.82.144:8888/upload\"\r\n",
                          (char *)"AT+HTTPPARA=\"CONTENT\",\"image\"\r\n",
                          (char *)"AT+HTTPDATA=5,10000\r\n", // POST the data, params (size as bytes, input latency time ms)
                          (char *)"DEBUG", // POST Data
                          (char *)"AT+HTTPACTION=1\r\n", // POST Session start
                          (char *)"AT+HTTPREAD\r\n" // Read the data
                        };
char *httpEnd[]       = { (char *)"AT+HTTPTERM\r\n" }; // Terminate HTTP Service

char *locationQuery[] = { (char *)"AT+CIPGSMLOC=1,1\r\n" }; // Base station Location: 1)type = 1, get longitude and latitude


void setup() {
  Serial.begin(9600);
  while(!Serial);
  serialSIM800.begin(9600);

  if(DEBUG) Serial.println("SETUP");
  delay(3000);
  setGprs();
  postData();
  terminateHttp();
  getLocation();
  endGprs();
}

void loop() {

}

void setGprs() {
  if(DEBUG) Serial.println("setGprs");
  writeToSim(gprsStart, sizeof(gprsStart)/sizeof(*gprsStart));
}

void endGprs() {
  if(DEBUG) Serial.println("endGprs");
  writeToSim(gprsEnd, sizeof(gprsEnd)/sizeof(*gprsEnd));
}

void postData() { // TODO Insert payload
  if(DEBUG) Serial.println("postData");
  writeToSim(httpStart, sizeof(httpStart)/sizeof(*httpStart));
}

void terminateHttp() {
  if(DEBUG) Serial.println("terminateHttp");
  writeToSim(httpEnd, sizeof(httpEnd)/sizeof(*httpEnd));
}

void getLocation() {
  if(DEBUG) Serial.println("getLocation");
  writeToSim(locationQuery, sizeof(locationQuery)/sizeof(*locationQuery));
  // TODO Scanf coordinates from responseBuffer
  Serial.println(readSim(responseBuffer, 5000));
}

void writeToSim(char *commands[], int size) {
  for(int i = 0; i < size; i++) {
    serialSIM800.write(commands[i]);
    if(DEBUG) { Serial.print("Writing: "); Serial.println(commands[i]); }
    delay(1000);
  }

  while(serialSIM800.available()) serialSIM800.read(); // Ignore written commands
}

// Read Sim response until timeout
char* readSim(char* buffer, int timeout) {
  unsigned long timeStart = millis();
  int charCount = 0;

  while(1) {
    if(serialSIM800.available()) buffer[charCount++] = serialSIM800.read();

    if(millis() - timeStart > timeout) break;
  }
  buffer[charCount++] = 0;
  return buffer;
}
