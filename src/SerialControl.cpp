#include "SerialControl.h"


SerialControl::SerialControl(void)
{
	connected = false;
	device_ready = false;
	baud_rate = 9600;
	sensor_value = 0;
	new_sensor_value = 0;
	is_black = false;

	responded = false;
	last_conn_time = -1000.0; //ensure that connection will be checked on startup
}

SerialControl::~SerialControl(void)
{
	if (connected || device_ready || thread.isRunning())
        disconnect();
}

void SerialControl::setup(int baud_rate) {
	this->baud_rate = baud_rate;
	startThread();
}

bool SerialControl::establish_connection() {
	//end all active connections
	init();

	//check all available serial devices
	vector <ofSerialDeviceInfo> device_infos = serial.getDeviceList();
	for (int i = 0; i < device_infos.size(); i ++)
    {
		connected = serial.setup(device_infos[i].getDeviceName(), baud_rate);
		
        if (connected)
		{
			//check if connected device is panorama controller
			if (!serial.writeByte('I'))
                break;
			
            cout << "Wrote I to " << device_infos[i].getDeviceName() << endl;
			float init_time = ofGetElapsedTimef();

			//wait for correct answer within specified time
			while (!device_ready)
			{
				if (serial.readByte() == 'R') {
					serial.drain();
					device_ready = true;
					ofLog() << "Succesfully connected to Panorama Controller on " << device_infos[i].getDeviceName();
					return true;
				}
				if (ofGetElapsedTimef() - init_time > 2.0) {
					device_ready = false;
					break;
				}
			}
		}
	}
	if (!device_ready) ofLogError() << "Panorama Controller could not be found.";
	return device_ready;
}


void SerialControl::init() {
    //fresh start
	serial.close();
	connected = false;
	device_ready = false;

}

void SerialControl::disconnect() {
	ofLogNotice() << "disconnecting serial controller ..";
	stopThread();
    serial.close();
    connected = false;
	ofLog() << "Serial device closed.";
}

void SerialControl::threadedFunction()
{
	while (thread.isRunning()) {
		if (lock()) {
			if (!device_ready) {
				ofLog() << "Panorama Controller not ready, try to connect";
				if (establish_connection()) {
					ofLog() << "Panorama Controller connected and ready!";
					responded = true;
					last_request_time = ofGetElapsedTimef();
				} else {
					ofLogError() << "Connection to PanoramaController failed";
					continue;
				}
			}

			//chekcing connection
			if (ofGetElapsedTimef() - last_request_time > 8.0f) {
				//send request
				serial.writeByte('Q');
				last_request_time = ofGetElapsedTimef();
				responded = false;
			}
			if (!responded && ofGetElapsedTimef() - last_request_time > 2.0f) {
				connected = false;
				device_ready = false;
				continue;
			}

			//recieving serial data

			char HEADER = 'H';
			short LF = 10;
			string str;
			string substring;
			bool new_button1_state = 0;
			bool new_button2_state = 0;

			do {

				str = getSerialString(serial, LF); //read until end of line
				if (str=="") continue;

				vector<string> data = ofSplitString(str, ",");
				if (data[0] == "H")
				{
					new_sensor_value = ofToInt(data[1]);
					new_button1_state = ofToBool(data[2]);
					new_button2_state = ofToBool(data[3]);
					new_is_black = ofToBool(data[4]);

					if (new_sensor_value != sensor_value) {
						sensor_value = new_sensor_value;
						ofNotifyEvent(E_sensor_value_changed, sensor_value, this);
					}
					if (new_button1_state == true && new_button1_state != button1_state)
					{
						button1_state = new_button1_state;
						ofNotifyEvent(E_button1_pressed, button1_state, this);
					}
					else if (new_button1_state == false && new_button1_state != button1_state)
					{
						button1_state = new_button1_state;
					}

					if (new_button2_state == true && new_button2_state != button2_state)
					{
						button2_state = new_button2_state;
						ofNotifyEvent(E_button2_pressed, button2_state, this);
					}
					else if (new_button2_state == false && new_button2_state != button2_state)
					{
						button2_state = new_button2_state;
					}

					if (new_is_black != is_black) {
						is_black = new_is_black;
						cout << "fade to " <<  is_black << endl;
						ofNotifyEvent(E_fade_to_dark, is_black, this);
					}
				}
				else if (data[0] == "A")
				{
					ofLogNotice() << "controller alive!";
					responded = true;
					//last_request_time = ofGetElapsedTimef();
				}
			} while (str!="");
			unlock();
		}
	}
}
