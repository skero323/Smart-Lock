#include <WiFi.h>
#include <WireGuard-ESP32.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Servo_ESP32.h>
#include <time.h>

//WiFi and WG init
WiFiServer server(80);
static WireGuard wg;
static HTTPClient httpClient;
char ssid[] = "Gaj_Wifi";
char password[] = "mojdom123";
char private_key[] = "0Pw/m0VlbsN1qkpNeGxnCRzQI4bcRnMHNyyWM7NXlWE=";
char public_key[] = "04pddjg8rzMmZbhXImwGBaOkWXXplmZM6aEcwfuzESw=";
char endpoint_address[] = "192.168.1.5";
IPAddress local_ip(192, 168, 200, 2);
int endpoint_port = 51820;
int povezani = 0;

static constexpr const uint32_t UPDATE_INTERVAL_MS = 5000;

//button init
#define BUTTON_PIN_BITMASK 0x200000000

//LED init
static const int ZAKLENJENO = 26;
static const int ODKLENJENO = 25;

//servo init
Servo_ESP32 servo;
static const int servoPin = 14;
int angle = 0;

//bluetooth init
#define SERVICE_UUID        "def8570d-1d17-4908-8bcb-082e56f566b9"
#define CHARACTERISTIC_UUID "f1226eca-6253-40e6-9c8b-1e0e5406e689"
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

//callbacks for bluetooth
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class Callbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        for (int i = 0; i < value.length(); i++) {
          if (value[i] == '1') {
            Serial.println("Odklepam");
            digitalWrite(ODKLENJENO, HIGH);
            digitalWrite(ZAKLENJENO, LOW);
            angle = 180;
            servo.write(angle);
            delay(15);
          }
          if (value[i] == '0') {
            Serial.println("Zaklepam");
            digitalWrite(ODKLENJENO, LOW);
            digitalWrite(ZAKLENJENO, HIGH);
            angle = 0;
            servo.write(angle);
            delay(15);
          }
        }
      }
    }
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  servo.attach(servoPin);
  servo.write(0);
  pinMode(ZAKLENJENO, OUTPUT);
  pinMode(ODKLENJENO, OUTPUT);

  //setting up WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while ( !WiFi.isConnected() ) {
    delay(1000);
  }
  server.begin();
  Serial.println(WiFi.dnsIP(0));
  Serial.println("Adjusting system time...");
  configTime(1 * 60 * 60, 0, "ntp.jst.mfeed.ad.jp", "ntp.nict.jp", "time.google.com");

  Serial.println("Connected. Initializing WireGuard...");
  wg.begin(
    local_ip,
    private_key,
    endpoint_address,
    public_key,
    endpoint_port);
  delay(4000);
  Serial.println("Done, setting up Bluetooth...");

  //setting up Bluetooth
  BLEDevice::init("ESP32 get noti from device");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pCharacteristic->setCallbacks(new Callbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  digitalWrite(ODKLENJENO, HIGH);
  Serial.println("Bluetooth is set, starting advertising...");
}

void loop() {
  //WiFi and WireGuard
  WiFiClient client = server.available();
  if (client) {  // if you get a client,
    Serial.printf("connected: %d", povezani);
    povezani++;
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
            client.print("         body { ");
            client.print("             text-align: center; ");
            client.print("             background-color: #033433; ");
            client.print("         } ");
            client.print("         .box{ ");
            client.print("             margin: 0; ");
            client.print("             position: absolute; ");
            client.print("             top: 50%; ");
            client.print("             left: 50%; ");
            client.print("             -ms-transform: translate(-50%, -50%); ");
            client.print("             transform: translate(-50%, -50%); ");
            client.print("         } ");
            client.print("         .gumbi{ ");
            client.print("             border: 0.1px solid black; ");
            client.print("             color: black; ");
            client.print("             padding: 41px 67px; ");
            client.print("             text-align: center; ");
            client.print("             text-decoration: none; ");
            client.print("             border-radius: 4px; ");
            client.print("             font-size: 18px; ");
            client.print("             margin: 5px; ");
            client.print("             font-weight: 550; ");
            client.print("         } ");
            client.print("         #odkleni { ");
            client.print("             background-color: #69f0ae; ");
            client.print("         } ");
            client.print("         #zakleni { ");
            client.print("             background-color: #ff5252; ");
            client.print("         } ");
            client.print("     </style> ");
            client.print(" </head> ");
            client.print(" <body> ");
            client.print("     <div class=\"box\"> ");
            client.print("         <div><a href=\"/26/\"><button class=\"gumbi\" id=\"odkleni\">Odkleni</button></a></div> ");
            client.print("         <div><a href=\"/25/\"><button class=\"gumbi\" id=\"zakleni\">Zakleni</button></a></div> ");
            client.print("     </div> ");
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
          Serial.println("Zaklepam");
          digitalWrite(ZAKLENJENO, HIGH);
          digitalWrite(ODKLENJENO, LOW);
          angle = 0;
          servo.write(angle);
        }
        if (currentLine.endsWith("GET /26/")) {
          Serial.println("Odklepam");
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

  //Bluetooth
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    digitalWrite(ZAKLENJENO, LOW);
  }
}
