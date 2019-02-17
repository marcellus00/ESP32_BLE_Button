#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <EEPROM.h>

#define EEPROM_SIZE 64
#define SERVICE_UUID "5e87bd74-b350-4a6a-ae36-bb33e30499af"
#define CHARATERISTIC_DATA_UUID "2e550f9f-c882-467a-b35a-c7f12ecae2b3"
#define CHARATERISTIC_BUTTON_UUID "12cd634c-2911-45c5-8c46-3d67628a88fd"
#define BUTTONPIN 25
#define DELAY 250

int _buttonPressed = LOW;
bool _messageReceivedComplete = false;
BLECharacteristic *pCharacteristicData;
BLECharacteristic *pCharacteristicButton;
std::string _message;
std::string _data = "<!DOCTYPE html><html><body><h1>BLE Button Admin Page</h1><p>Hello</p></body></html>";

class ServerReadCallbacks: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
      _message = pCharacteristic->getValue();
      _messageReceivedComplete = true;
    }
};

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  EEPROM.begin(EEPROM_SIZE);
  String deviceName = EEPROM.readString(0);
  Serial.println("Name " + deviceName);
  int nameLength = deviceName.length();
  if(nameLength == 0 ||
      nameLength == 1 && 
        deviceName == "0")
  {
    deviceName = "BLE Button";
  }

  BLEDevice::init(deviceName.c_str());
  BLEServer *pServer = BLEDevice::createServer();
  //pServer->setCallbacks(new EchoServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristicData = pService->createCharacteristic(
                          CHARATERISTIC_DATA_UUID,
                          BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
                        );
  pCharacteristicData->addDescriptor(new BLE2902());
  pCharacteristicData->setCallbacks(new ServerReadCallbacks());

  pCharacteristicButton = pService->createCharacteristic(
                            CHARATERISTIC_BUTTON_UUID,
                            BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
                          );
  pCharacteristicButton->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  Serial.println("Characteristic defined.");
  pinMode(BUTTONPIN, INPUT_PULLDOWN);
  pCharacteristicButton->setValue(_buttonPressed);
  pCharacteristicData->setValue(_data);
}

void loop()
{
  delay(DELAY);
  int currentState = digitalRead(BUTTONPIN);
  if (_buttonPressed != currentState)
  {
    _buttonPressed = currentState;
    pCharacteristicButton->setValue(_buttonPressed);
    pCharacteristicButton->notify();
  }

  if (_messageReceivedComplete)
  {
    _messageReceivedComplete = false;
    int length = _message.length();
    if (length > 0)
    {
      if (length > 32)
      {
        _message.erase(32, length - 32);
      }
      if(_message == "0")
        _message = "0 ";
      Serial.println("Rename");
      EEPROM.writeString( 0, _message.c_str() );
      EEPROM.commit();
      ESP.restart();
    }
  }
}
