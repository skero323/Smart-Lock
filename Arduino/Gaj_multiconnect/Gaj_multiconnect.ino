#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Servo_ESP32.h>
#include <time.h>
#define BUTTON_PIN_BITMASK 0x200000000
RTC_DATA_ATTR int bootCount = 0;

const int LED2 = 25;
const int LED = 26;

Servo_ESP32 servo;
static const int servoPin = 14;
int angle =0;
int angleStep = 2;

int angleMin =0;
int angleMax = 180;

int pos = 0;

int currTime = 0;
int prevTime = 0;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID        "def8570d-1d17-4908-8bcb-082e56f566b9"
#define CHARACTERISTIC_UUID "f1226eca-6253-40e6-9c8b-1e0e5406e689"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic *pCharacteristic){
    std::string value = pCharacteristic->getValue();

    if(value.length() > 0){
      Serial.println("************");
      Serial.print("New value: ");
      
      for(int i = 0; i < value.length(); i++){
        Serial.print(value[i]);
        if(value[i] == '1'){
          angle = 180;
          servo.write(angle);
          Serial.println(angle);
          delay(15);
          }
         if(value[i] == '0'){
           angle = 0;
           servo.write(angle);
           Serial.println(angle);
           delay(15);
          }
        }
      }
      
      Serial.println();
      Serial.print("************");
    }
};

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

int getTimePassed(){
  float mils = millis()/1000;
  int sec = mils + 0.5;
  return sec;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  prevTime = getTimePassed();
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  print_wakeup_reason();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_32,1);
  servo.attach(servoPin);
  servo.write(0);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2, LOW);
  // Create the BLE Device
  BLEDevice::init("ESP32 get noti from device");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  digitalWrite(LED2, HIGH);
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    currTime = getTimePassed();
    if(currTime - prevTime > 30){
      prevTime = currTime;
      digitalWrite(LED, HIGH);
      Serial.println("Going to sleep now");
      delay(1000);
      esp_deep_sleep_start();
    }
      
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
        digitalWrite(LED2, LOW);
    }
}
