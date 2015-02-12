#include "PPlayerApp.h"

//--------------------------------------------------------------
void PPlayerApp::setup(){
	vsync = false;
	ofBackground(0, 0, 0);
	ofEnableDepthTest();
	ofEnableAlphaBlending();
	ofDisableArbTex();
	//ofSetFrameRate(30);
	//ofSetFullscreen(true);
	ofSetVerticalSync(true);

	//init variables
	h = 100;
	x = y = z = -100;
	width = height = length = 16000;
	fov = 70.0f;
	rotation = 0.0f;
	rotation_step = 1.0f;
	center.set(0,0, 0);
	x = -width/2;
	y = -height/2;
	z = -length/2;
    
	//initViewPorts();
	
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

	//Arduino stuff
	potValue = "N/A";

	serial.listDevices();
	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
	serial.setup(deviceList[1].getDeviceName(),9600);

	cout << "===" << endl;
	cout << ofToString(InitVSync()) << endl;
	cout << ofToString(SetVSync(true)) << endl;
	cout << "===" << endl;
}

//--------------------------------------------------------------
void PPlayerApp::update(){
	camera.setFov(fov);
	updateSerial();
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
	int sensorValue, button1, button2;

	// Receive String from Arduino

	string str;
	string substring;
	do {
		str = getSerialString(serial,'\n');
		//read until end of line
		if (str=="") continue;
		/*
		for(int i = 0; i < str.length(); i++) {
			printf("%c",str[i]);
		}
		printf("\n");
		*/
		size_t pos = str.find_first_of(',');
		substring = str.substr(0, pos);
		sensorValue = atoi(substring.c_str());
		//cout << sensorValue << endl;
		rotation = ofMap(sensorValue,0,1023,180,270);
		fade_factor = ofMap(sensorValue,0,1023,0,1);
		//		}
	} while (str!="");
}

//--------------------------------------------------------------
void PPlayerApp::digitalPinChanged(const int & pinNum) {
	//since every button press means two changes on the pin, we need to differentiate between press and release
	//we do that by checking the pinNum for ARD_HIGH or ARD_LOW
	//ARD_HIGH means pressed
	//ARD_LOW means released

	switch (pinNum)
	{
	case 7:
		if (arduino.getDigital(pinNum) == ARD_HIGH) {
			ofToggleFullscreen();
		}
		break;
	case 8:
		if (arduino.getDigital(pinNum) == ARD_HIGH) {
			cout << ofToString(ofGetElapsedTimeMillis()) << ": " << pinNum << " | pressed" << endl;
			cycleTextures();
			cout << "Texture index: " << texture_index << endl;
		}
		break;
	default:
		if (arduino.getDigital(pinNum) == ARD_HIGH) {
			cout << "No action specified for pin " << pinNum << "|pressed." << endl;
		}
		else if (arduino.getDigital(pinNum) == ARD_LOW) {
			cout << "No action specified for pin " << pinNum << "|released." << endl;
		}
		break;
	}
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
		msg += "Field of View \t\t" + ofToString(fov) + "\n";
		msg += "Rotation \t\t" + ofToString(rotation) + "\n";
		msg += "Rotation step \t\t " + ofToString(rotation_step) + "\n";
		msg += "Potentiometer \t\t " + potValue + "\n";
		msg += "Panorama \t\t" + ofToString(texture_index) + "\n";
		msg += "FPS: \t\t" + ofToString(ofGetFrameRate());
	} else {
		msg = "No Textures loaded!";
	}

	//draw a mask
	glEnable(GL_BLEND);
	ofMatrixMode(OF_MATRIX_PROJECTION);
	/*ofPushMatrix();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0,0,0,fade_factor);
	ofRect(ofRectangle(0,0,ofGetWidth(),ofGetHeight()););
	ofPopMatrix();*/

	ofPushMatrix();
	glBlendFunc(GL_ZERO,GL_SRC_COLOR);
	mask_image.draw(0.0f, 0.0f, ofGetWidth(), ofGetHeight());
	ofPopMatrix();
	glDisable(GL_BLEND);

	ofDrawBitmapStringHighlight(msg, 50, 50);
}

//--------------------------------------------------------------
void PPlayerApp::keyPressed(int key){
	if (key == OF_KEY_LEFT)
	{
		rotation_step -= 0.1f;
	}
	else if (key == OF_KEY_RIGHT)
	{
		rotation_step += 0.1f;
	}
	else if ( key == OF_KEY_UP )
		fov += 10.0;
	else if (key == OF_KEY_DOWN)
		fov -= 10.0f;
	else if ( key == 'f'){
		ofToggleFullscreen();
		//initViewPorts();
	}
	else if (key == 'a')
		rotation -=rotation_step;
	else if (key == 'd')
		rotation +=rotation_step;
	else if (key == 'r')
		arduino.sendReset();
}

//--------------------------------------------------------------
void PPlayerApp::keyReleased(int key){

}

//--------------------------------------------------------------
void PPlayerApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void PPlayerApp::mouseDragged(int x, int y, int button){

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

	//	if (all_panoramas.size() > 0)
	//		current_panorama = (ofImage**) &all_panoramas[texture_index];
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