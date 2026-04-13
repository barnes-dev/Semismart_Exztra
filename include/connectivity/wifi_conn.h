#include "../../user_config.h"
#if BLUETOOTH_WIFI_ENABLED

#ifndef CONNECTIVITY_WIFI_H
#define CONNECTIVITY_WIFI_H

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include "WiFi.h"
#include <Preferences.h>
#include "../core/system_config.h"
#include "bluetooth_conn.h"

class WiFiExztra {
	public:
	void WiFi_setup();
	void WiFi_search();
	void setSSIDcredentials();
	void setSSIDselected(bool selected);
	bool getSSIDselected();
	int isNumberofNetworksFound();
	String getyourSSID(int index);
	String getTheSSID();
	uint8_t getWiFiStatus();
	String getPSW();
	void setPSWmemory(const String& psw);
	void setSSIDmemory(const String& ssid);
	void clearSSIDList();
	void connectToSSID();
	void connectAsync();
	bool hasSavedCredentials();
	void setSSIDselectedMemory(bool selected);
	bool getSSIDselectedMemory();
	String getIPAddress();
	String getMACAddress();
	void resetWiFi();
};

extern WiFiExztra wifiExztra;

#endif // CONNECTIVITY_WIFI_H

#endif // BLUETOOTH_WIFI_ENABLED