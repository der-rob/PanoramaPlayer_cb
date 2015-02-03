#include "PPlayerApp.h"

//--------------------------------------------------------------
void PPlayerApp::setup(){
	//init variables
	h = 100;
	x = y = z = -100;
	width = height = length = 400;

	fov = 90.0f;

	rotation = 0.0f;
    rotation_step = 1.0f;

	center.set(ofGetWidth()*.5, ofGetHeight()*.5, 0);

    ofDisableArbTex();
    ofEnableDepthTest();

    //initial loading of all textures
	texture_index = 0;
	scanTextureFolder();

    //ofEnableTextureEdgeHack();

    glCullFace(GL_BACK);

    width = height = length = 16000;

//    ofAddListener(powermate.tengoInfo, this, &panoGLApp::powermateEvent);
    camera.setGlobalPosition(0.0f, 0.0f, 0.0f);
    camera.setFarClip(32000);
    camera.setFov(fov);
    camera.disableMouseInput();

    center.set(0,0, 0);
    x = -width/2;
    y = -height/2;
    z = -length/2;

	//Arduino stuff
	potValue = "N/A";

	arduino.connect("COM5", 57600);

	ofAddListener(arduino.EInitialized, this, &PPlayerApp::setupArduino);
	bArduinoSetup = false;
}

//--------------------------------------------------------------
void PPlayerApp::update(){
	 camera.setFov(fov);
	 updateArduino();
}

//--------------------------------------------------------------
void PPlayerApp::setupArduino(const int &version) {
	cout << "setting up arduino" << endl;
	//ofRemoveListener(arduino.EInitialized, this, &PPlayerApp::setupArduino);
	bArduinoSetup = true;

	ofLogNotice() << arduino.getFirmwareName();
	ofLogNotice() << "firmata v" << arduino.getMajorFirmwareVersion() << "." << arduino.getMinorFirmwareVersion();

	//arduino smoothing
	index = total = average = 0;
	for (int thisReading = 0; thisReading < numReadings; thisReading++)
		readings[thisReading] = 0;

	arduino.sendAnalogPinReporting(0, ARD_ANALOG);
	arduino.sendDigitalPinMode(7,ARD_INPUT);
	arduino.sendDigitalPinMode(8,ARD_INPUT);

	ofAddListener(arduino.EAnalogPinChanged, this, &PPlayerApp::analogPinChanged);
	ofAddListener(arduino.EDigitalPinChanged, this, &PPlayerApp::digitalPinChanged);
}

//--------------------------------------------------------------
void PPlayerApp::updateArduino() {
	arduino.update();
}

//--------------------------------------------------------------
void PPlayerApp::analogPinChanged(const int & pinNum) {
	total = total - readings[index];
	readings[index] = arduino.getAnalog(pinNum);
	total = total + readings[index];
	index++;
	index = index % numReadings;
	average = total / numReadings;

	potValue = ofToString(average);
	rotation = ofMap(average, 0, 1023, 30, 330);
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
		cout << ofToString(ofGetElapsedTimeMillis()) << ": " << pinNum << " | pressed" << endl;
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

        msg += "Field of View \t\t" + ofToString(fov) + "\n";
        msg += "Rotation \t\t" + ofToString(rotation) + "\n";
        msg += "Rotation step \t\t " + ofToString(rotation_step) + "\n";
        msg += "Potentiometer \t\t " + potValue + "\n";
        msg += "Panorama \t\t" + ofToString(texture_index);
	} else {
        msg = "No Textures loaded!";
	}

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
    else if ( key == 'f')
        ofToggleFullscreen();
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
