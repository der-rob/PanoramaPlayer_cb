#pragma once

#include <iostream>
#include "ofMain.h"
#include "ofThread.h"

class SerialControl : public ofThread
{
public:
	SerialControl(void);
	~SerialControl(void);

	void setup(int baud_rate);

	void init();
	void disconnect();
	void update();
	void threadedFunction();

	bool isConnected() { return connected; }

	ofEvent<bool> E_button1_pressed;
	ofEvent<bool> E_button2_pressed;
	ofEvent<int> E_sensor_value_changed;
	ofEvent<bool> E_fade_to_dark;

private:
	bool establish_connection();

	float last_conn_time;
	float last_request_time;
	int sensor_value;
	bool button1_state;
	bool button2_state;
	bool is_black;
	bool new_is_black;
	int new_sensor_value;
	int baud_rate;

protected:
	ofSerial serial;
	bool connected;
	bool device_ready;
	bool responded;

private:
	//some function to handle the strings which we get from serial
	string trimStringRight(string str) {
		size_t endpos = str.find_last_not_of(" \t\r\n");
		return (string::npos != endpos) ? str.substr( 0, endpos+1) : str;
	}
	// trim trailing spaces
	string trimStringLeft(string str) {
		size_t startpos = str.find_first_not_of(" \t\r\n");
		return (string::npos != startpos) ? str.substr(startpos) : str;
	}

	string trimString(string str) {
		return trimStringLeft(trimStringRight(str));
	}

	string getSerialString(ofSerial &the_serial, char until) {
		static string str;
		stringstream ss;
		char ch;
		int ttl=1000;
		ch=the_serial.readByte();
		while (ch > 0 && ttl >= 0 && ch!=until) {
			ss << ch;
			ch=the_serial.readByte();
		}
		str+=ss.str();
		if (ch==until) {
			string tmp=str;
			str="";
			return trimString(tmp);
		} else {
			return "";
		}
	}
};

