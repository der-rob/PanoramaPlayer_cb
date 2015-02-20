#include "PPlayerApp.h"

//--------------------------------------------------------------
void PPlayerApp::setup(){
	vsync = false;
	bControllerConnected = false;
	ofBackground(0, 0, 0);
	ofEnableDepthTest();
	ofDisableArbTex();
	ofSetFullscreen(true);
	show_stats = false;

	//init variables
	h = 100;
	x = y = z = -100;
	width = height = length = 16000;
	fov = 70.0f;
	rotation = 0.0f;
	center.set(0,0, 0);
	x = -width/2;
	y = -height/2;
	z = -length/2;
	
	

	//initViewPorts();
	shader.load("shaders\\binoculars.vert","shaders\\binoculars.frag");
	ofLogLevel(ofLogVerbose);

	//initial loading of all textures
	texture_index = 0;
	scanTextureFolder();
	mask_image.allocate(1920,1080, OF_IMAGE_COLOR);
	mask_image.loadImage("mask.jpg");
	black_fade_mask = ofRectangle(0,0,ofGetWidth(),ofGetHeight());
	fade_factor=0.5;

	camera.setGlobalPosition(0.0f, 0.0f, 0.0f);
	camera.setFarClip(32000);
	camera.setFov(fov);
	camera.disableMouseInput();

	

	cout << "===" << endl;
	cout << ofToString(InitVSync()) << endl;
	cout << ofToString(SetVSync(true)) << endl;
	cout << "===" << endl;
	
	//Arduino stuff
	potValue = "N/A";
	
	bControllerConnected = establishControllerConnection(serial); 
}
//--------------------------------------------------------------
bool PPlayerApp::establishControllerConnection(ofSerial & _serial) { 
	//check serial devices
	//_serial.listDevices();
	vector <ofSerialDeviceInfo> deviceList = _serial.getDeviceList();
	//connect to last device in list, should be the mikro controller
	if (deviceList.size() > 0)
		if (_serial.setup(deviceList.back().getDeviceName(),9600)) {
			//check if connected device is panorama controller
			_serial.writeByte('I');
			float init_time = ofGetElapsedTimef();
			//wait for correct answer within specified time
			bool connected = false;
			
			while (!connected)
			{
				if (_serial.readByte() == 'R')
					connected = true;
				if (ofGetElapsedTimef() - init_time > 2)
					break;
			}
			return true;
		}
		else return false;
}
//--------------------------------------------------------------
void PPlayerApp::update(){
	camera.setFov(fov);
	if (bControllerConnected)
		updateSerial();
	else
	{
		if (ofGetElapsedTimef() - last_conn_try > 1)
			bControllerConnected = establishControllerConnection(serial);
	}
}

//--------------------------------------------------------------
string PPlayerApp::trimStringRight(string str) {
	size_t endpos = str.find_last_not_of(" \t\r\n");
	return (string::npos != endpos) ? str.substr( 0, endpos+1) : str;
}
// trim trailing spaces
string PPlayerApp::trimStringLeft(string str) {
	size_t startpos = str.find_first_not_of(" \t\r\n");
	return (string::npos != startpos) ? str.substr(startpos) : str;
}
string PPlayerApp::trimString(string str) {
	return trimStringLeft(trimStringRight(str));
}
string PPlayerApp::getSerialString(ofSerial &the_serial, char until) {
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
//--------------------------------------------------------------
void PPlayerApp::updateSerial() {
	//check if mc is still online
	if (ofGetElapsedTimef() - send_request_time > 8) {
		serial.writeByte('Q');
		send_request_time = ofGetElapsedTimef();
		b_responded = false;
	}
	if (!b_responded && ofGetElapsedTimef() - send_request_time > 2)
		bControllerConnected = false;

	char HEADER = 'H';
	short LF =10;
	int sensorValue;
	// Receive String from Arduino
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
			sensorValue = ofToInt(data[1]);
			potValue = data[1];
			rotation = ofMap(sensorValue,0,1023,180,270);

			new_button1_state = ofToBool(data[2]);
			new_button2_state = ofToBool(data[3]);

			if (new_button1_state == true && new_button1_state != button_state_1)
			{
				cout << "Button 1 pressed" << endl;
				button1pressed();
				button_state_1 = new_button1_state;
			}
			else if (new_button1_state == false && new_button1_state != button_state_1)
			{
				cout << "Button 1 released" << endl;
				button_state_1 = new_button1_state;
			}
			
			if (new_button2_state == true && new_button2_state != button_state_2)
			{
				cout << "Button 2 pressed" << endl;
				button2pressed();
				button_state_2 = new_button2_state;
			}
			else if (new_button2_state == false && new_button2_state != button_state_2)
			{
				cout << "Button 2 released" << endl;
				button_state_2 = new_button2_state;
			}
		}
		else if (data[0] == "A")
		{
			cout << "controller alive!" << endl;
			b_responded = true;
			bControllerConnected = true;
			last_send_q_time = ofGetElapsedTimeMillis() % 65535;
		}
	} while (str!="");
}

//--------------------------------------------------------------
void PPlayerApp::draw(){
	string msg ="";
	
	if (texture_index >= 0) {
		/*
		for (int v = 0; v < viewports.size(); v++)
		{*/
		//camera.begin(viewports[v]);
		//texture_index = v;	

		camera.begin();
		ofPushMatrix();
		ofTranslate(center.x, center.y,0);
		ofRotate(rotation, 0, 1, 0);

		all_panoramas[texture_index][0].getTextureReference().bind();
		all_panoramas[texture_index][0].getTextureReference().unbind();

		// Draw Front side
		all_panoramas[texture_index][0].getTextureReference().bind();

		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z+length);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y+height, z+length);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y+height, z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y, z+length);
		glEnd();
		all_panoramas[texture_index][0].getTextureReference().unbind();

		// Draw Back side
		all_panoramas[texture_index][1].getTextureReference().bind();
		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y, z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y+height, z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y+height, z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, z);
		glEnd();
		all_panoramas[texture_index][1].getTextureReference().unbind();

		// Draw up side
		all_panoramas[texture_index][2].getTextureReference().bind();
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y+height, z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y+height, z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y+height, z+length);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y+height, z);
		glEnd();
		all_panoramas[texture_index][2].getTextureReference().unbind();

		// Draw down side
		all_panoramas[texture_index][3].getTextureReference().bind();
		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z+length);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y, z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y, z);
		glEnd();
		all_panoramas[texture_index][4].getTextureReference().unbind();

		// Draw Left side
		all_panoramas[texture_index][4].getTextureReference().bind();
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y, z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y, z+length);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y+height, z+length);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y+height, z);
		glEnd();
		all_panoramas[texture_index][4].getTextureReference().unbind();

		// Draw Right side
		all_panoramas[texture_index][5].getTextureReference().bind();
		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y+height, z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y+height, z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z);
		glEnd();
		all_panoramas[texture_index][5].getTextureReference().unbind();

		ofPopMatrix();
		camera.end();

		//}
		//show stats and stuff
		if (show_stats)
		{
			//check if controller is connected
			if (!bControllerConnected)
				msg += "Sensor controller is not connected!\n";

			msg += "Field of View \t\t" + ofToString(fov) + "\n";
			msg += "Rotation \t\t" + ofToString(rotation) + "\n";
			msg += "Potentiometer \t\t " + potValue + "\n";
			msg += "Panorama \t\t" + ofToString(texture_index) + "\n";
			msg += "FPS: \t\t" + ofToString(ofGetFrameRate());
		} 
	} else {
		msg = "No Textures loaded!";
	}

	ofPushMatrix();
	ofMatrixMode(OF_MATRIX_PROJECTION);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//shader
	shader.begin();
	shader.setUniform1f("border_size",0.01);
	shader.setUniform1f("disc_radius", 0.5);
	shader.setUniform4f("disc_color",1.0,1.0,1.0,1.0);
	shader.setUniform2f("disc_center", 0.5,0.5);
	shader.setUniform1f("alpha", fade_factor);
	float aspect_ratio = (float)ofGetWidth() / (float)ofGetHeight();
	shader.setUniform1f("aspect_ratio", aspect_ratio);
	//glColor4f(0,0,0,fade_factor);
	all_panoramas[texture_index][5].getTextureReference().bind();
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(ofGetWidth(), 0);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(ofGetWidth(), ofGetHeight());
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0, ofGetHeight());
	glEnd();
	all_panoramas[texture_index][5].getTextureReference().unbind();
	shader.end();


	//draw a mask
	ofPushMatrix();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO,GL_SRC_COLOR);
	mask_image.draw(0.0f, 0.0f, ofGetWidth(), ofGetHeight());
	ofPopMatrix();
	glDisable(GL_BLEND);

	ofDrawBitmapStringHighlight(msg, 50, 50);
}
//--------------------------------------------------------------
void PPlayerApp::button1pressed() {
	cycleTextures();
}
//--------------------------------------------------------------
void PPlayerApp::button2pressed() {
	ofToggleFullscreen();
}
//--------------------------------------------------------------
void PPlayerApp::keyPressed(int key){

	if ( key == OF_KEY_UP )
		fov += 10.0;
	else if (key == OF_KEY_DOWN)
		fov -= 10.0f;
	else if ( key == 'f'){
		ofToggleFullscreen();
		//initViewPorts();
	}
	else if (key == 's')
		show_stats = ! show_stats;
}

//--------------------------------------------------------------
void PPlayerApp::keyReleased(int key){

}

//--------------------------------------------------------------
void PPlayerApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void PPlayerApp::mouseDragged(int x, int y, int button){
	//rotation = ofMap(x,0, ofGetWidth(),0,360);
}

//--------------------------------------------------------------
void PPlayerApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void PPlayerApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void PPlayerApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void PPlayerApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void PPlayerApp::dragEvent(ofDragInfo dragInfo){

}

//--------------------------------------------------------------
void PPlayerApp::scanTextureFolder() {
	//get the texture names per folder, get these names from an xml file which has to bee in each folder
	vector< vector<string> > all_panorama_filenames;
	string picPath= "picture_data";
	ofDirectory picDir(picPath);
	int picDirSize = picDir.listDir();
	picDir.sort();

	// go through all files in picture_data directory
	for (int i = 0; i < picDirSize; i++){
		//check if entry is directory (the one where the pictures are supossed to be)
		if (picDir.getFile(i).isDirectory() == 1){
			//there is one subdirectory for each panorama
			string picSubDirPath = picDir.getFile(i).getAbsolutePath();
			ofDirectory picSubDir(picSubDirPath);
			picSubDir.listDir();
			// find config.xml
			ofFile configFile;
			string configFilePath = picSubDirPath + "\\config.xml";
			configFile.open(configFilePath);
			if ( configFile.exists()) {
				ofXml panorama;
				ofBuffer buffer = configFile.readToBuffer();
				panorama.loadFromBuffer(buffer.getText());
				//check if xml file contains entry panorama
				string key_name = "panorama";
				if (panorama.setTo(key_name)) {
					int picCount = panorama.getNumChildren();
					vector<string> this_panorama_filenames;
					for (int j = 0; j < picCount; j++){
						string sub_key_name = "picture["+ofToString(j)+"][@id="+ofToString(j)+"]/name";
						if (panorama.exists(sub_key_name)) {
							string name = picSubDirPath + "\\" + panorama.getValue(sub_key_name);
							this_panorama_filenames.push_back(name);
							//cout << name << endl;
						} else {
							cout << "Failure while reading picture keys. ";
							cout << "Configfile might be corrupted. Please check!" << endl;
							cout << "\tKey " << sub_key_name << " does not exsist" << endl;
							break;
						}
					}
					if (!this_panorama_filenames.empty())
						all_panorama_filenames.push_back(this_panorama_filenames);
				} else {
					// error handling
					cout << key_name << " not found!" << endl;
					cout << "Skip this directory." << endl;
				}

			} else {
				// error handling
				// configfile not found
				cout << "No config.xml found in " + configFilePath << endl;
			}
		}
	}

	//load the textures
	if (all_panorama_filenames.size() >= 1) {
		cout << "Panoramas to load: " << all_panorama_filenames.size() << endl;
		for (int i = 0; i < all_panorama_filenames.size(); i++) {
			vector <ofImage> this_panorama;
			for (int j = 0; j < all_panorama_filenames[i].size(); j++) {
				ofImage picture;
				cout << all_panorama_filenames[i][j].c_str() << endl;
				if (picture.loadImage(all_panorama_filenames[i][j].c_str()))
					this_panorama.push_back(picture);
			}
			//only add if all pictures are loaded correctly
			if (this_panorama.size() == 6)
			{
				all_panoramas.push_back(this_panorama);
				cout << "Loaded " << all_panoramas.size() << endl;
			}
		}
	}
	if (all_panoramas.size() >=1)
		texture_index = 0;
	else texture_index = -1;
}

//--------------------------------------------------------------
void PPlayerApp::cycleTextures(){
	texture_index++;
	//to avoid bad accesses
	texture_index = texture_index % all_panoramas.size();
}
/*
//--------------------------------------------------------------
bool PPlayerApp::initViewPorts() {
float _width = ofGetWidth();
float _height = ofGetHeight();
int _x,_y;
ofVec2f midpoint;
float angle_rad = 72*(PI/180);
float radius = _width/(2*tan(angle_rad));
midpoint = ofVec2f(_width/2.0, _height + radius);
cout << midpoint << endl;
// calculate midpoint for pentagon
viewports.clear();
for (int i = 0; i<5; i++)
{
ofRectangle rect = ofRectangle(i * _width, 0, _width, _height);
viewports.push_back(rect);
}

return true;
}
*/