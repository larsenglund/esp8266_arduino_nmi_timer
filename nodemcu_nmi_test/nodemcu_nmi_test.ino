#define DEBUG

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif

extern "C" {
  #include "ets_sys.h"
  #include "os_type.h"
  #include "osapi.h"
  #include "gpio.h"
  #include "user_interface.h"
  #include "mem.h"
  #include "hw_timer.h"
  void ets_delay_us(uint16 us);
}

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "FS.h"

#define ZERO_CROSS_PIN 4
#define TRIAC_PIN 5
#define TRIAC_PIN_NATIVE BIT5
#define MAX_DIM_LEVEL 10000

const char* ssid     = "Huset";
const char* password = "";

volatile unsigned int dimlevel = MAX_DIM_LEVEL/2; // 0-MAX_DIM_LEVEL with 0=off
volatile unsigned int dim_slot = MAX_DIM_LEVEL/2; // 0-MAX_DIM_LEVEL with 200=off
unsigned int dim_counter = MAX_DIM_LEVEL+1;

int inByte = 0;         // incoming serial byte
#define IN_DATA_MAX_LEN 64
char inData[IN_DATA_MAX_LEN]; // Buffer to store incoming commands from serial port
int inDataIdx = 0;

ESP8266WebServer server(80);


void handleRoot() {
  //digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  //digitalWrite(led, 0);
}

void handleNotFound(){
  //digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  //digitalWrite(led, 0);
  Serial.println("File Not Found");
}


void ICACHE_RAM_ATTR onTimerISR(){
  if(GPIP(TRIAC_PIN)){
    GPOC = (1 << TRIAC_PIN);//low
    timer1_write(600);//120us
  } else GPOS = (1 << TRIAC_PIN);//high
}

void ICACHE_RAM_ATTR onPinISR(){
  //if(GPIP(ZERO_CROSS_PIN)){
  //  GPOS = (1 << TRIAC_PIN);//high
  //  timer1_write(5000 * 5);
  //}
  GPOS = (1 << TRIAC_PIN);//high
  timer1_write(dimlevel * 5);
}


/*void zero_cross_isr() 
{
  dim_counter = 0;
}*/

/*
 // TODO: Check optotriac & triac datasheet for miminum time
volatile bool state = false;
void ICACHE_RAM_ATTR blink_gpio(void)
{
  //if ((T1C & ((1 << TCAR) | (1 << TCIT))) == 0) TEIE &= ~TEIE1;//edge int disable
  //T1I = 0;

  // to make ISR compatible to Arduino AVR model where interrupts are disabled
  // we disable them before we call the client ISR
  //uint32_t savedPS = xt_rsil(15); // stop other interrupts

  ///your code here
  if (dim_counter == dim_slot) {
    gpio_output_set(0, TRIAC_PIN_NATIVE, TRIAC_PIN_NATIVE, 0); // LOW
    //os_delay_us(12);
    //gpio_output_set(TRIAC_PIN_NATIVE, 0, TRIAC_PIN_NATIVE, 0); // HIGH
  }
  if (dim_counter == dim_slot+10) {
    gpio_output_set(TRIAC_PIN_NATIVE, 0, TRIAC_PIN_NATIVE, 0); // HIGH
  }
  dim_counter++;

  //xt_wsr_ps(savedPS);
}
*/

void requestWebpage() {
  const char* host = "www.apastyle.org";

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/";
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  int n=0;
  while(client.available()){
    n++;
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.print("Num lines: ");
  Serial.println(n);
  
  Serial.println();
  Serial.println("CLOSING connection");
}


void espInit() {
  SPIFFS.begin();
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  Serial.println("SPIFFS");
  Serial.print("totalBytes: ");
  Serial.println(fs_info.totalBytes);
  Serial.print("usedBytes: ");
  Serial.println(fs_info.usedBytes);

  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
      Serial.print(dir.fileName());
      File f = dir.openFile("r");
      Serial.println(f.size());
  }


  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
    }
  }
  Serial.println("");

  
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
}


void setup() {
  Serial.begin(115200);
  Serial.println("setup()");


  espInit();


  pinMode(13, OUTPUT);
  pinMode(ZERO_CROSS_PIN, INPUT);
  pinMode(14, OUTPUT);
  pinMode(TRIAC_PIN, OUTPUT);


  Serial.println("Triac test");
  digitalWrite(TRIAC_PIN, HIGH);
  delay(500);
  Serial.println("ON test");
  digitalWrite(TRIAC_PIN, LOW);
  delay(1000);
  Serial.println("OFF test");
  digitalWrite(TRIAC_PIN, HIGH);
  delay(1000);


  //attachInterrupt(digitalPinToInterrupt(ZERO_CROSS_PIN), zero_cross_isr, RISING); // TODO: verify with scope if this should be on FALLING or RISING
  timer1_attachInterrupt(onTimerISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  attachInterrupt(ZERO_CROSS_PIN, onPinISR, FALLING);//RISING);

  interrupts(); // Enable interrupts

  /*
  hw_timer_init(FRC1_SOURCE, 1);
  //hw_timer_init(NMI_SOURCE, 1);
  hw_timer_set_func(blink_gpio);
  hw_timer_arm(25);
*/
  
  Serial.println("Entering main loop..");
}


void loop() {
  Serial.print("*");
  for (int n=0; n<5000; n+=10) {
    if (Serial.available() > 0) {
      inByte = Serial.read();
      if (inByte != '\n') {
        inData[inDataIdx++] = inByte;
      }
      else {
        // Process message when new line character is recieved
        inData[inDataIdx] = '\0';
        DBG( Serial.print("Arduino Received: ") );
        DBG( Serial.println(inData) );
        DBG( Serial.print(inDataIdx) );
        DBG( Serial.println(" bytes") );
        unsigned int tmp_level = strtoul(inData, NULL, 0);
        if (tmp_level > MAX_DIM_LEVEL) tmp_level = MAX_DIM_LEVEL;
        dimlevel = tmp_level;
        dim_slot = MAX_DIM_LEVEL - dimlevel;
        DBG( Serial.print("Dimlevel: ") );
        Serial.println(dimlevel);
        inDataIdx = 0; // Clear recieved buffer
      }
    }
    delay(10);
    server.handleClient();
  }
  requestWebpage();
}
