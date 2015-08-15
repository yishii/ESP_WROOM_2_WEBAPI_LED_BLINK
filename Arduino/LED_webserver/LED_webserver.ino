#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>
 
const char* ssid = "********";
const char* password = "********";

#define PIXEL_PIN    14    // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 30
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

MDNSResponder mdns;

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}

void handleNotFound(){
  digitalWrite(led, 1);
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
  digitalWrite(led, 0);
}

int target_r;
int target_g;
int target_b;
int curr_r;
int curr_g;
int curr_b;
  
void drive_led(){
  int val;
  digitalWrite(led, 1);

  val = server.arg(0).toInt();
  if(server.argName(0) == "r"){
    Serial.print("Red ");
    target_r = val;
  } else if (server.argName(0) == "g"){
    Serial.print("Green ");
    target_g = val;
  } else if (server.argName(0) == "b"){
    Serial.print("Blue ");
    target_b = val;
  }
  Serial.println(val);
  //setPixelColor(r,g,b);  
  digitalWrite(led, 0);
  server.send(200, "text/plain", "OK");
}

int led_target(int curr,int target)
{
  if(curr == target){
    return curr;
  }
  if(curr < target){
    return curr+1;
  }
  if(curr > target){
    return curr-1;
  }
}
void handleLed()
{
  int i;
  static int last_updated = 0;

  if(last_updated < millis()){
    last_updated = millis() + 10;

    curr_r = led_target(curr_r,target_r);
    curr_g = led_target(curr_g,target_g);
    curr_b = led_target(curr_b,target_b);

    for(i=1;i<PIXEL_COUNT;i++){
      strip.setPixelColor(i-1,strip.getPixelColor(i));
    }
    setPixelColor(curr_r,curr_g,curr_b); 
    strip.show();
  }
}

void setPixelColor(int r,int g,int b){
  strip.setPixelColor(PIXEL_COUNT-1,strip.Color(r,g,b));
}
 
void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  
  server.on("/", handleRoot);
  server.on("/led", drive_led);
  
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}
 
void loop(void){
  server.handleClient();
  handleLed();
} 
