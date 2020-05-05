/*
 
Web server mas valor de corriente
 */
//A0 sensor voltaje
//D7 dht11
//A1 Sensor corriente
#include <SPI.h>
#include <Ethernet.h>
#include "EmonLib.h"                   // Include Emon Library

#include "DHT.h"
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
int offset =0;// set the correction offset value Si 0VAC a la entrada entonces marco 0
double v_ute; 
EnergyMonitor emon1;                   // Create an instance


byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 100);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  dht.begin();
  emon1.current(1, 76.9);             // Current: input pin, calibration. Aless 76.9 Fiera 60.6
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
 
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          double Irms = emon1.calcIrms(1480);  // Calculate Irms only
          int h = dht.readHumidity();// Lee la humedad
          int t= dht.readTemperature();//Lee la temperatura
          int volt = analogRead(A0);// read the input
          double v_read = map(volt,0,1023, 0, 2500) + offset;// map 0-1023 to 0-2500 and add correction offset
          if(v_read==0){
            v_ute=0;
          }
          else {
            v_read /=100;// divide by 100 to get the decimal values
            /*Uso un modelo lineal ax+b, segun mis medidas a=10 y b=68  */
            v_ute=10*v_read + 68;
          }
          client.println(Irms);          // Irms
          client.println(":");
          client.println(h);
          client.println(":");
          client.println(t);
          client.println(":");
          client.println(v_ute);
          client.println("<br />");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
