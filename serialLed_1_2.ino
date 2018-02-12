/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */
//protype change
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

SoftwareSerial swSer(13, 14);
/* Set these to your desired credentials. */
const char *ssid = "Ledserial";
const char *password = "ptl12345";
int RST=12;

void(* resetFunc) (void) = 0;//declare reset function at address 0

 IPAddress ip(192,168,0,4); 
  IPAddress gateway(192,168,0,1);
  IPAddress subnet(255,255,255,0);
// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  swSer.begin(9600);
  delay(10);

pinMode(RST,OUTPUT);
 digitalWrite(RST, HIGH);
  // digitalWrite(RST, LOW);
 // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
   WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  String retString="";
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  int counter = 0;
  while(!client.available()){
    delay(1);
    counter++;
    if (counter >1000){
      Serial.println("returning as client is not available");
      client.flush();
      client.print("Error");
      break;
    }
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
// int tempIndex = req.indexOf("GET");
// int startIdx = req.indexOf("/",tempIndex+2);
// Serial.print(startIdx);
  // Match the request
//  int val;


   //checkreset(req);
   
   if(req.indexOf("/reset")!= -1){
    retString="reset function";
    digitalWrite(RST, LOW);
    delay(3000);
    digitalWrite(RST, HIGH);
//    setup();
//resetFunc(); //call reset 
   }
  else if (req.indexOf("/parmComd") != -1){
    int startIdx = req.indexOf("Comd=")+5;
    int endIdx = req.indexOf("HTTP",(startIdx))-1;
    String tempCommand = req.substring(startIdx,endIdx);
    String  command = "";
    for (int i=0;i<tempCommand.length();i++){
      if (tempCommand.charAt(i)=='%'){
        i++;
        int convChar = 0;
        if (tempCommand.charAt(i) >='0' && tempCommand.charAt(i) <='9'){
          convChar = tempCommand.charAt(i) - '0';
        }
        else if (tempCommand.charAt(i) >='A' && tempCommand.charAt(i) <='F'){
          convChar = tempCommand.charAt(i) - 'A'+10;
        }
        /*Serial.print("convChar1=");
        Serial.print((char)convChar);
        Serial.print("=");
        Serial.print((int)convChar);
        Serial.println("=");*/
        convChar *= 16;
        /*Serial.print("convChar2=");
        Serial.print((char)convChar);
        Serial.print("=");
        Serial.print((int)convChar);
        Serial.println("=");*/
        i++;
        /*Serial.print("char=");
        Serial.print((char)tempCommand.charAt(i));
        Serial.print("=");
        Serial.print((int)tempCommand.charAt(i));
        Serial.println("=");*/
        if (tempCommand.charAt(i) >='0' && tempCommand.charAt(i) <='9'){
          convChar += tempCommand.charAt(i) - '0';
        }
        else if (tempCommand.charAt(i) >='A' && tempCommand.charAt(i) <='F'){
          convChar += tempCommand.charAt(i) - 'A'+10;
        }
        command += (char)convChar;
        /*Serial.print("convChar2=");
        Serial.print((char)convChar);
        Serial.print("=");
        Serial.print((int)convChar);
        Serial.println("=");*/
        int convInt = convChar;
      }
      else {
        command += tempCommand.charAt(i);
      }
      Serial.println(command);
    }
//    delay(1000);
    retString = sendCommand(command);
    Serial.print("ret=");
    Serial.print(retString);
    Serial.println("=");
    
  }
  else {
    
    Serial.println("invalid request");
    client.stop();
    return;
  }

  // Set GPIO2 according to the request
//  digitalWrite(2, val);
//  
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
  s += retString;
  // Prepare the response
  //String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
  //s += (val)?"high":"low";
  //s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");
delay(2000);
  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

String sendCommand(String inputCommand){
  String outputCommand="";
//  char inputCmdChar[] = inputCommand.toCharArray();
  Serial.print("Comm Sent=");
  for(int i=0;i<inputCommand.length();i++){
    swSer.write(inputCommand.charAt(i));
    Serial.write(inputCommand.charAt(i));
  }
  while (swSer.available() > 0) {
    char ch=swSer.read();
    Serial.write(ch);
    outputCommand += ch;
    delay(1);
  }
  //Serial.print(inputCommand);
  Serial.println("=");
  long cmdStartTime = millis();
  //delay(3000);
  int serAvailable =0;
  serAvailable = swSer.available();
  int counter = 0;
  while (serAvailable<=0 && millis() - cmdStartTime < 5000){
    if (counter%100==0){
      Serial.print(serAvailable);
      Serial.print(" \t");
      Serial.println(millis() - cmdStartTime);
      serAvailable = swSer.available();
    }
    counter++;
    delay(1);
  }
  Serial.print("\n from led panel=");
  while (swSer.available() > 0) {
    char ch=swSer.read();
    Serial.write(ch);
    outputCommand += ch;
    delay(1);
  }
  Serial.println("=");
  Serial.println(serAvailable);
  if (!serAvailable){
    return "Invalid Entry. Not Prosessed";
  }
  return outputCommand;
}



void checkreset(String req)
{
  if(req == "reset")
{
  digitalWrite(RST, LOW); 
 delay(3000);
 digitalWrite(RST, HIGH);
 delay(3000);
 resetFunc(); //call reset 
 
}
}

