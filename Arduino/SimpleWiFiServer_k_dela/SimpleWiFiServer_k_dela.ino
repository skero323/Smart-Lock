#include <WiFi.h>
#include <Servo_ESP32.h>

static const int ZAKLENJENO = 26;
static const int ODKLENJENO = 25;
static const int servoPin = 14;
int angle =0;
int angleStep = 5;
int angleMin =0;
int angleMax = 180;

const char* ssid     = "Gaj_Wifi";
const char* password = "mojdom123";

Servo_ESP32 servo;
WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    servo.attach(servoPin);
    pinMode(ZAKLENJENO, OUTPUT);
    pinMode(ODKLENJENO, OUTPUT);
    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();

}

int value = 0;

void loop(){
 WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print(" <!DOCTYPE html> ");
            client.print(" <html lang=\"en\"> ");
            client.print(" <head> ");
            client.print("     <meta charset=\"UTF-8\"> ");
            client.print("     <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"> ");
            client.print("     <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> ");
            client.print("     <title>Document</title> ");
            client.print("     <style> ");
            client.print("         body{ ");
            client.print("             text-align: center; ");
            client.print("             background-color: #3a639c; ");
            client.print("         } ");
            client.print("         h1{ ");
            client.print("             background-color: #df5c29; ");
            client.print("             border-radius: 15px; ");
            client.print("             padding: 14px 40px; ");
            client.print("         } ");
            client.print("         .button{ ");
            client.print("             border-radius: 8px; ");
            client.print("             padding: 14px 40px; ");
            client.print("             background-color: #df5c29; ");
            client.print("             border: 1px solid black; ");
            client.print("             border-radius: 15px; ");
            client.print("             font-size: 20px; ");
            client.print("             margin: 10px; ");
            client.print("             cursor:crosshair; ");
            client.print("         } ");
            client.print("     </style> ");
            client.print(" </head> ");
            client.print(" <body> ");
            client.print("     <h1>DoorLock</h1> ");
            client.print("     <br> ");
            client.print("     <div><a href=\"/25/\"><button class=\"button\">LOCK</button></a></div> ");
            client.print("     <br> ");
            client.print("     <div><a href=\"/26/\"><button class=\"button\">UNLUCK</button></a></div> ");
            client.print(" </body> ");
            client.print(" </html> ");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /25/")) {
          digitalWrite(ZAKLENJENO, HIGH);
          digitalWrite(ODKLENJENO, LOW);
          angle = 0;
          servo.write(angle);
        }
        if (currentLine.endsWith("GET /26/")) {
          digitalWrite(ZAKLENJENO, LOW);
          digitalWrite(ODKLENJENO, HIGH);
          angle = 180;
          servo.write(angle);
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
