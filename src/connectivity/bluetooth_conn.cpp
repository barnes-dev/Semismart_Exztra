#include "../../user_config.h"
#if BLUETOOTH_WIFI_ENABLED
#include "../../include/connectivity/bluetooth_conn.h"

BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool wifiSetupStarted = false;
uint8_t txValue = 0;
uint32_t timeoutTimer = 0;
char deviceName[32] = { 0 };
std::string rxValue;

/*
* Callback class to handle Bluetooth connection events. It updates the 
* deviceConnected flag when a device connects or disconnects, which can 
* be used to manage the Bluetooth connection state and control advertising behavior.
*/
class MyServerCallbacks : public BLEServerCallbacks {
	void onConnect(BLEServer* pServer) {
		viewManager.setView(ViewID::BT_CONNECTED);
		deviceConnected = true;
	};

	void onDisconnect(BLEServer* pServer) {
		deviceConnected = false;
	}
};

/*
* Callback class to handle writes to the RX characteristic.
* When a new value is written by the connected Bluetooth device,
*/
class MyCallbacks : public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic* pCharacteristic) {
		rxValue = pCharacteristic->getValue();
	}
};

/*
* This function retrieves the last received value from the Bluetooth characteristic.
* It returns an empty string if no new value has been received since the last call.
*/
std::string BTExztra::getReceivedValue() {
	static std::string lastValue;
	if (rxValue != lastValue) {
		lastValue = rxValue;
	}
	// Clear rxValue after reading to avoid processing the same value multiple times
	rxValue.clear();
	return lastValue;
}

/*
* Create an instance of the BTExztra class to manage Bluetooth functionality across the program.
*/
BTExztra btExztra;

/*
* This function gets the connected device name, if available.
* It returns an empty string if no device is connected or if the device name cannot be retrieved.
*/
std::string BTExztra::getDeviceName() {
	if (deviceConnected) {
		return std::string(deviceName);
	}
	return "";
}


/*
* Check if a Bluetooth device is currently connected. This can be used to determine whether to show the 
* connected screen or to allow sending notifications.
*/
bool BTExztra::isDeviceConnected() {
	return deviceConnected;
}

/*
* Check if the Bluetooth connection has timed out due to inactivity. 
* If the device is connected, reset the timeout timer.
* If the device has been inactive for more than 5 minutes, stop advertising 
* to save power and return true to indicate a timeout.
* If the device is still active or has not yet reached the timeout threshold, 
* return false to indicate that the connection is still valid.
*/
bool BTExztra::isConnectTimeout() {
	if (deviceConnected)
		timeoutTimer = millis();              // reset timeout timer on activity
	if (millis() - timeoutTimer > 300000) {  // 5 minute timeout for inactivity
		pServer->getAdvertising()->stop();    // stop advertising to save power
		return true;                          // Bluetooth timeout - continue to start the main interface. This allows the system to be used even if Bluetooth is not working.
	}
	else {
		return false;
	}
}

/*
* Reset the Bluetooth connection timeout timer and restart advertising to allow new connections.
*/
void BTExztra::resetConnectTimeout() {
	timeoutTimer = millis();
	pServer->getAdvertising()->start();    // start advertising to allow new connections
}

/*
* Turn off Bluetooth advertising and disconnect any connected devices. 
* This can be used to save power when Bluetooth is not needed, or to force a new connection 
* if the current one is not working properly.
*/
void BTExztra::resetBluetooth() {
	pServer->getAdvertising()->stop();    // stop advertising to save power
	if (deviceConnected) {
		pServer->disconnect(0);           // disconnect any connected device
	}
}

/*
* This function initializes the Bluetooth functionality by creating a BLE server, service, and characteristics.
* It sets up the necessary callbacks for handling connections and received data, 
* starts the service, and begins advertising to allow devices to connect.
*/
void BTExztra::startBTSetup() {

	// Create the BLE Device
	BLEDevice::init("Semismart Exztra");

	// Create the BLE Server
	pServer = BLEDevice::createServer();
	pServer->setCallbacks(new MyServerCallbacks());

	// Create the BLE Service
	BLEService* pService = pServer->createService(SERVICE_UUID);

	// Create a BLE Characteristic
	pTxCharacteristic = pService->createCharacteristic(
		CHARACTERISTIC_UUID_TX,
		BLECharacteristic::PROPERTY_NOTIFY);

	pTxCharacteristic->addDescriptor(new BLE2902());

	BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
		CHARACTERISTIC_UUID_RX,
		BLECharacteristic::PROPERTY_WRITE);

	pRxCharacteristic->setCallbacks(new MyCallbacks());

	// Start the service
	pService->start();

	// Start advertising
	pServer->getAdvertising()->start();
	//Serial.println("Waiting a client connection to notify...");
}

/*
* This function handles the Bluetooth connection state and data transmission. 
* It checks for incoming serial data and sends it via BLE notifications if a device is connected. 
* It also manages the connection state by updating the view when a device connects or disconnects, 
* and by restarting advertising when a device disconnects.
*/
void BTExztra::connectReceive() {
	// Only send notifications if a device is connected
	//if (deviceConnected) {
	//	// If there's serial input, read a line and send it via BLE
	//	if (Serial.available()) {
	//		String line = Serial.readStringUntil('\n');  // read until newline
	//		line.trim();                                 // remove CR/LF/whitespace

	//		if (line.length() > 0) {
	//			// send the text string as a notification
	//			std::string out(line.c_str());
	//			pTxCharacteristic->setValue(out);
	//			pTxCharacteristic->notify();
	//			//Serial.print("Sent via BLE: ");
	//			//Serial.println(line);
	//		}
	//		delay(10);  // bluetooth stack will go into congestion, if too many packets are sent
	//	}
	//}
	/*
	* Static variable to track the time of the last disconnection, used for managing advertising behavior.
	*/
	static uint32_t disconnectTime = 0;
	static uint32_t screenDelayTimer = 0;
	// disconnecting
	if (!deviceConnected && oldDeviceConnected) {
		viewManager.setView(ViewID::BT_SCANNING);
		if (millis() - disconnectTime > 1000) {  // 500 millisecond timeout for inactivity
			pServer->startAdvertising();           // restart advertising
			//		Serial.println("start advertising");
			oldDeviceConnected = deviceConnected;
		}
	}
	// connecting
	if (deviceConnected && !oldDeviceConnected) {
		//		Serial.println("Connected to BT-device");
		if (!wifiSetupStarted) {
			wifiExztra.WiFi_setup();
			wifiSetupStarted = true;
		}
		if (millis() - screenDelayTimer > 2000) {
			screenDelayTimer = 0;  // reset the screen delay timer after showing the connected screen for 3 seconds
			viewManager.setView(ViewID::WIFI_SCANNING);
			oldDeviceConnected = deviceConnected;
		}
	}

	if (deviceConnected && oldDeviceConnected) {
		disconnectTime = millis();  // record the time of disconnection
	}

	if (!deviceConnected && !oldDeviceConnected) {
		viewManager.setView(ViewID::BT_SCANNING);
		screenDelayTimer = millis();  // record the time of disconnection
	}
}


#endif  // BLUETOOTH_ENABLED