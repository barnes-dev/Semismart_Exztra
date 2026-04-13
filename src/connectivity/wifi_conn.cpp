/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is based on the Arduino WiFi Shield library, but has significant changes as newer WiFi functions are supported.
 *  E.g. the return value of `encryptionType()` different because more modern encryption is supported.
 */
#include "../../user_config.h"
#if BLUETOOTH_WIFI_ENABLED
#include "../../include/connectivity/wifi_conn.h"

uint32_t lastWiFiScanTime = 0;

String wifiSSIDList[MAX_WIFI_NETWORKS];
int numberOfNetworksFound = 0;
int ssidIndex[MAX_WIFI_NETWORKS];
bool SSIDselected = false;
bool wifiConnected = false;

WiFiExztra wifiExztra;
inline Preferences WIFI_EEPROM;

/*
* This function initializes the WiFi module by setting it to station mode and disconnecting 
* from any previously connected access points.
* It ensures that the WiFi module is ready for scanning and connecting to new networks. 
* The delay after disconnecting allows the module to reset its state before starting a new scan.
*/
void WiFiExztra::WiFi_setup() {

	// Set WiFi to station mode and disconnect from an AP if it was previously connected.
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);

	//Serial.println("Setup done");
}

/*
* This function performs a WiFi scan to find available networks. It limits the scanning frequency 
* to every 10 seconds to save resources.
* The function first checks if enough time has passed since the last scan, then it scans 
* for networks and updates the list of found SSIDs without overwriting previously stored ones.
* It also checks for user input via Bluetooth to select an SSID from the list of found networks 
* and stores the selection in memory for later use when connecting to WiFi.
*/
void WiFiExztra::WiFi_search() {
	// Wait for 10 seconds before repeating the scan.
	if (millis() - lastWiFiScanTime < 10000) {
		// Limit WiFi scanning to every 10 seconds to save resources
		return;
	}
	lastWiFiScanTime = millis();

	wifiConnected = false;

	//	Serial.println("Scan start");
	// WiFi.scanNetworks will return the number of networks found.

	int n = WiFi.scanNetworks();

	// Do not clear previously stored indices � preserve written slots.
	// Count currently filled slots.
	int filled = 0;
	for (int i = 0; i < MAX_WIFI_NETWORKS; ++i) {
		if (ssidIndex[i] == 1 && wifiSSIDList[i].length() > 0) {
			++filled;
		}
	}
	numberOfNetworksFound = filled;

	/*
	* Iterate over scan results and add only new SSIDs into free slots.
	* Never overwrite a slot that has been written (ssidIndex == 1).
	*/
	for (int i = 0; i < n; ++i) {
		String ssid = WiFi.SSID(i);

		// Skip empty SSIDs
		if (ssid.length() == 0) continue;

		// Check if SSID already exists anywhere in the stored list
		bool exists = false;
		for (int j = 0; j < MAX_WIFI_NETWORKS; ++j) {
			if (ssidIndex[j] == 1 && wifiSSIDList[j] == ssid) {
				exists = true;
				break;
			}
		}
		if (exists) continue;

		// Find first free slot (never overwrite an occupied index)
		int freeSlot = -1;
		for (int j = 0; j < MAX_WIFI_NETWORKS; ++j) {
			if (ssidIndex[j] == 0) {
				freeSlot = j;
				break;
			}
		}

		// If no free slot, stop adding
		if (freeSlot == -1) break;

		// Store SSID into the free slot and mark it filled
		ssidIndex[freeSlot] = 1;
		wifiSSIDList[freeSlot] = ssid;
		++numberOfNetworksFound;
	}
	// Delete the scan result to free memory for code below.
	WiFi.scanDelete();

	/*
	* This section waits for input fromthe user via Bluetooth, which selects the index 
	* of the SSID from the list of found networks. The user is expected to send a number 
	* corresponding to the desired network (e.g., "1" for the first network, "2" for the second, etc.). 
	* The code then checks if the input is valid and if an SSID has not already been selected, 
	* it stores the selected SSID in memory for later retrieval when connecting to WiFi.
	*/
	std::string inputFromBT = btExztra.getReceivedValue();
	inputFromBT[sizeof(inputFromBT) - 1] = '\0';
	if (!inputFromBT.empty()) {
		int index = std::stoi(inputFromBT);
		index -= 1;  // Adjust for 0-based index
		if (index >= 0 && index < numberOfNetworksFound) {
			SSIDselected = getSSIDselectedMemory();  // Update SSIDselected from memory to ensure it reflects the stored state
			if (!SSIDselected) {
				SSIDselected = true;
				setSSIDselectedMemory(SSIDselected);  // Store the selection state in memory
				setSSIDmemory(wifiSSIDList[index]);  // Store the selected SSID in memory for later retrieval when connecting to WiFi
			}
		}
	}
}

/*
* This function manages the process of setting the SSID credentials for WiFi connection. 
* It first displays the WiFi selection screen, then waits for the user to input the password 
* via Bluetooth. Once the password is received, it stores it in memory and attempts to connect to the selected SSID.
*/
void WiFiExztra::setSSIDcredentials() {
	viewManager.setView(ViewID::WIFI_SELECT);
	viewManager.draw();  // keep splash animation alive

	std::string inputPSW;
	// If inputPSW is empty, wait for the user to input the password via Bluetooth and store it in inputPSW.
	while (inputPSW.length() == 0) {
		inputPSW = btExztra.getReceivedValue();
		setPSWmemory(inputPSW.c_str());
		viewManager.draw();  // keep splash animation alive
		if (!btExztra.isDeviceConnected()) break;
	}
	if (!btExztra.isDeviceConnected()) return;
	connectToSSID();
}

/*
* This function attempts to connect to the WiFi network using the stored SSID and password.
* It includes a retry mechanism that tries to connect for a certain number of attempts with a delay between each attempt.
* If the connection is successful, it sets the wifiConnected flag to true and resets the 
* Bluetooth connection to free up resources.
* If the connection fails after the specified number of attempts, it disconnects from WiFi 
* to stop further connection attempts.
*/
void WiFiExztra::connectToSSID() {
//	viewManager.setView(ViewID::WIFI_CONNECTED);
//	viewManager.draw();  // keep splash animation alive

	String wifi_psw = getPSW();
	String myssid = getTheSSID();

	WiFi.begin(myssid.c_str(), wifi_psw.c_str());

	int tryDelay = 1000;
	int numberOfTries = 15;

	while (WiFi.status() != WL_CONNECTED) {
		wifiConnected = false;
		delay(tryDelay);
		if (numberOfTries <= 0) {
			// Serial.print("[WiFi] Failed to connect to WiFi!");
			// Use disconnect function to force stop trying to connect
			WiFi.disconnect();
			return;
		}
		else {
			numberOfTries--;
		}
	}

	if (WiFi.status() == WL_CONNECTED) {
		wifiConnected = true;
		btExztra.resetBluetooth();  // Reset Bluetooth connection after successful WiFi connection to free up resources and avoid conflicts
	}
}

/*
* This function fetch the IP-address of the connected WiFi network, 
*/
String WiFiExztra::getIPAddress() {
	if (WiFi.status() == WL_CONNECTED) {
		return WiFi.localIP().toString();
	}
	return "";
}


/*
* This function fetches the current state of the WiFi system, 
* which can be used to determine whether the device is currently connected 
* to a WiFi network.
*/
uint8_t WiFiExztra::getWiFiStatus() {
	return WiFi.status();
}

/*
* If credentials are stored and the device is not connected, 
* attempt to connect to the WiFi network using the stored credentials.
*/
void WiFiExztra::connectAsync() {
	viewManager.setView(ViewID::WIFI_CONNECTED);
	viewManager.draw();
	SSIDselected = getSSIDselectedMemory();  // Update SSIDselected from memory to ensure it reflects the stored state
	if (SSIDselected && !wifiConnected) {
		connectToSSID();
	}
}

/*
* If credentials are stored.
*/
bool WiFiExztra::hasSavedCredentials() {
	SSIDselected = getSSIDselectedMemory();  // Update SSIDselected from memory to ensure it reflects the stored state
	if (SSIDselected) {
		WIFI_EEPROM.begin("wifi_storage", true);
		String stored_ssid = WIFI_EEPROM.getString("WIFI_SSID", "");
		String stored_psw = WIFI_EEPROM.getString("WIFI_PSW", "");
		WIFI_EEPROM.end();
			stored_ssid.trim();
		stored_psw.trim();
		return !stored_ssid.isEmpty() && !stored_psw.isEmpty();
	}
	return false;
}

/*
* Reset the WiFi connection by disconnecting from the current network and clearing stored credentials.
*/
void WiFiExztra::resetWiFi() {
	WiFi.disconnect();
	WIFI_EEPROM.begin("wifi_storage", false);
	WIFI_EEPROM.remove("WIFI_SSID");
	WIFI_EEPROM.remove("WIFI_PSW");
	WIFI_EEPROM.end();
	SSIDselected = false;
	setSSIDselectedMemory(SSIDselected);  // Update memory to reflect that no SSID is selected
}

/*
* Return the number of networks found during the last scan. 
* This can be used to display the list of available networks to the user.
*/
int WiFiExztra::isNumberofNetworksFound() {
	return numberOfNetworksFound;
}

/*
* Return the SSID of the network at the given index in the list of found networks.
*/
String WiFiExztra::getyourSSID(int index) {
	if (index >= 0 && index < numberOfNetworksFound) {
		return wifiSSIDList[index];
	}
	return "";
}

/*
* Clear the list of found SSIDs and reset the indices. This can be called before starting 
* a new scan to ensure that old results are not displayed.
*/
void WiFiExztra::clearSSIDList() {
	for (int i = 0; i < MAX_WIFI_NETWORKS; ++i) {
		ssidIndex[i] = 0;
		wifiSSIDList[i] = "";
	}
	numberOfNetworksFound = 0;
}


/*
* Set whether an SSID has been selected by the user. This can be used to control the flow of the program,
* e.g., to determine when to prompt for the password or when to attempt a connection.
* and store the selection state in memory for later retrieval when connecting to WiFi or when 
* displaying the selected network to the user.
*/
void WiFiExztra::setSSIDselected(bool selected) {
	SSIDselected = selected;
	setSSIDselectedMemory(selected);  // Store the selection state in memory for later retrieval
}

/*
* Return whether an SSID has been selected by the user. This can be used to 
* determine whether to show the password input screen or to attempt a connection.
* This value is stored in memory and can be retrieved across different runs of the 
* program, allowing the program to remember whether the user has selected an SSID even after a restart.
*/
bool WiFiExztra::getSSIDselected() {
	SSIDselected = getSSIDselectedMemory();  // Update SSIDselected from memory to ensure it reflects the stored state
	return SSIDselected;
}

/*
* Return the currently selected SSID from memory. This can be used to display 
* the selected network to the user or to attempt a connection.
*/
String WiFiExztra::getTheSSID() {
	SSIDselected = getSSIDselectedMemory();  // Update SSIDselected from memory to ensure it reflects the stored state
	if (SSIDselected) {
		WIFI_EEPROM.begin("wifi_storage", true);
		String this_ssid = WIFI_EEPROM.getString("WIFI_SSID", "");
		WIFI_EEPROM.end();
		this_ssid.trim();
		return this_ssid;
	}
	return "";
}

/*
* Return the currently stored password for the selected SSID from memory. This can be used to
* attempt a connection to the WiFi network using the stored credentials. If no SSID is selected, 
* it returns an empty string.
*/
String WiFiExztra::getPSW() {
	SSIDselected = getSSIDselectedMemory();  // Update SSIDselected from memory to ensure it reflects the stored state
	if (SSIDselected) {
		WIFI_EEPROM.begin("wifi_storage", true);
		String this_psw = WIFI_EEPROM.getString("WIFI_PSW", "");
		WIFI_EEPROM.end();
		this_psw.trim();
		return this_psw;
	}
	return "";
}

/*
* Return the value of SSIDselected from memory, which indicates whether the user has selected an SSID. This can be used to
* determine whether to show the password input screen or to attempt a connection.
*/
bool WiFiExztra::getSSIDselectedMemory() {
	WIFI_EEPROM.begin("wifi_storage", true);
	bool selected = WIFI_EEPROM.getBool("SSID_SELECTED", false);
	WIFI_EEPROM.end();
	return selected;
}

/*
* Store the given password in memory for the currently selected SSID. This allows the program to
* retrieve the password later when attempting to connect to the WiFi network. If no SSID is selected,
* this function does nothing to avoid storing a password without an associated SSID.
*/
void WiFiExztra::setPSWmemory(const String& wifi_psw) {
	if (SSIDselected) {
		WIFI_EEPROM.begin("wifi_storage", false);
		WIFI_EEPROM.putString("WIFI_PSW", wifi_psw);
		WIFI_EEPROM.end();
	}
}

/*
* Store the given SSID in memory as the currently selected network. This allows the program to
* retrieve the selected SSID later when attempting to connect to the WiFi network or when displaying 
* the selected network to the user.
* If no SSID is selected, this function does nothing to avoid storing an SSID without user selection.
*/
void WiFiExztra::setSSIDmemory(const String& wifi_ssid) {
	if (SSIDselected) {
		WIFI_EEPROM.begin("wifi_storage", false);
		WIFI_EEPROM.putString("WIFI_SSID", wifi_ssid);
		WIFI_EEPROM.end();
	}
}

/*
* Store the value of SSIDselected in memory. This can be used to remember whether 
* the user has selected an SSID across different runs of the program,
* allowing the program to restore the previous state when it is restarted.
*/
void WiFiExztra::setSSIDselectedMemory(bool selected) {
	WIFI_EEPROM.begin("wifi_storage", false);
	WIFI_EEPROM.putBool("SSID_SELECTED", selected);
	WIFI_EEPROM.end();
}

/*
* Fetch the MAC address of the WiFi module. 
* This can be used for identification purposes or to display the device's MAC address to the user.
*/
String WiFiExztra::getMACAddress() {
	// Implementation to fetch MAC address
	return WiFi.macAddress();
}

#endif  // BLUETOOTH_WIFI_ENABLED