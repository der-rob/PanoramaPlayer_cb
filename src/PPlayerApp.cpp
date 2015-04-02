#include "PPlayerApp.h"

//--------------------------------------------------------------
void PPlayerApp::setup(){
	ofBackground(0, 0, 0);
	ofEnableDepthTest();
	ofDisableArbTex();

	//load settings from file
	if (!loadSettings("settings.xml"))
	{
		cout << "Something went wrong loading settings. Continue loading defaults." << endl;
		loadDefaultSettings();
	}

	currentTime = currentTime = ofGetHours() * 60 + ofGetMinutes();

	if (currentTime > shutdown_time) {
        ofLogError() << "shutdown time is set to earlier this day.";
        ofLogError() << "Please check or disable shutdown timer";
        ofLogNotice() << "App is going down now.";
        ofExit();
    }


	ofSetFullscreen(fullscreen);

	//enabling vsync by hand since oF version is not working correctly
	InitVSync();
	SetVSync(true);

	ofLogLevel(ofLogVerbose);

	//init graphics stuff
	pCube = PanoramaCube(16000, ofVec3f(0,0,0));
	camera.setGlobalPosition(0.0f, 0.0f, 0.0f);
	camera.setFarClip(32000);
	camera.setFov(fov);
	camera.disableMouseInput();
	viewing_direction = 0.0f;

	//initial loading of all textures
	scanTextureFolder();
	panorama_index = 0;

	//init shader stuff
	binocular = Binocular("shaders\\binoculars.vert","shaders\\binoculars.frag");

	//init controller connection
	serial_control.setup(115200);
	potValue = "N/A";
	last_sensor_value = 0;

	ofAddListener(serial_control.E_button1_pressed, this, &PPlayerApp::button1pressed);
	ofAddListener(serial_control.E_button2_pressed, this, &PPlayerApp::button2pressed);
	ofAddListener(serial_control.E_sensor_value_changed, this, &PPlayerApp::sensor_value_changed);
	ofAddListener(serial_control.E_fade_to_dark, this, &PPlayerApp::fade_to);
}

//--------------------------------------------------------------
void PPlayerApp::exit() {
    serial_control.stopThread();
}

//--------------------------------------------------------------
void PPlayerApp::update(){
	float dt = ofGetLastFrameTime();

	if (animatable.getAnimation() != NULL)
		animatable.updateAnimation(dt);

	if (last_recieved_fading_state != current_fading_state && animatable.getAnimation() == NULL)
	{
		if (last_recieved_fading_state && (animatable.getAnimationState() == BRIGHT))
		{
			animatable.createAnimation(0.0, 1.0, blending_speed, GO_DARK);
		}
		else if (!last_recieved_fading_state && (animatable.getAnimationState() == DARK))
		{
			panorama_index = new_panorama_index;
			animatable.createAnimation(1.0, 0.0, blending_speed, GO_BRIGHT);
		}
		current_fading_state = last_recieved_fading_state;
	}

	currentTime = ofGetHours() * 60 + ofGetMinutes();
	if (use_shutdown_timer)
        if(currentTime >= shutdown_time)
        {
            cout << "Application is going down now." << endl;
            ofExit();
        }
}

//--------------------------------------------------------------
void PPlayerApp::draw(){
	string msg ="";

	camera.begin();
	pCube.render(all_panoramas[panorama_index], viewing_direction);
	camera.end();

	//show stats and stuff
	if (panorama_index >= 0) {
		if (show_stats)
		{
			//check if controller is connected
			/*if (!bControllerConnected)
			msg += "Sensor controller is not connected!\n";*/

			msg += "Field of View \t\t" + ofToString(fov) + "\n";
			msg += "viewing_direction \t" + ofToString(viewing_direction) + "\n";
			msg += "Encoder value \t\t " + potValue + "\n";
			msg += "Panorama \t\t" + ofToString((int)panorama_index) + " " + ofToString((int)new_panorama_index) + "\n";
			msg += "FPS: \t\t\t" + ofToString(ofGetFrameRate()) + "\n";
			msg += "Blending Speed: \t" + ofToString(blending_speed) + "\n";
			msg += "Rotation Offset: \t" + ofToString(rotation_offset)  + "\n";
			msg += "System Time: \t\t" + ofToString(ofGetHours(),2,'0') + ":" + ofToString(ofGetMinutes(),2,'0') + ":" + ofToString(ofGetSeconds(),2,'0') + "\n";
			msg += "Shutdown Time int: \t" + ofToString((shutdown_time / 60),2,'0') + ":" + ofToString((shutdown_time % 60), 2, '0') + "\n";

		}
	} else {
		msg = "No Textures loaded!";
	}

	binocular.set_alpha(animatable.getValue());
	binocular.render();

	ofDrawBitmapStringHighlight(msg, 50, 50);
}
//--------------------------------------------------------------
void PPlayerApp::button1pressed(bool &state) {
	if (animatable.getAnimation() == NULL)
		cycleTextures_up();

}
//--------------------------------------------------------------
void PPlayerApp::button2pressed(bool &state) {
	if (animatable.getAnimation() == NULL)
		cycleTextures_down();
}
void PPlayerApp::sensor_value_changed(int &value) {
	if (abs(last_sensor_value - value) > 3)
		last_sensor_value = value;

	viewing_direction = ofMap(last_sensor_value,0,16384,0,360) * rotation_scale + rotation_offset;
	potValue = ofToString(last_sensor_value);
}


//--------------------------------------------------------------
void PPlayerApp::keyPressed(int key){

	if ( key == OF_KEY_UP ) {
		if (animatable.getAnimation() == NULL) {
			cycleTextures_up();
		}
	}
	else if (key == OF_KEY_DOWN) {
		if (animatable.getAnimation() == NULL) {
			cycleTextures_down();
		}
	}
	else if ( key == 'f') {
		ofToggleFullscreen();
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
	viewing_direction = ofMap(x,0, ofGetWidth(),0,360);
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
bool PPlayerApp::scanTextureFolder() {
	//provide some information during the loading process
	string info = "";

	//get the texture names per folder, get these names from an xml file which has to bee in each folder
	vector< vector<string> > all_panorama_filenames;
	string picPath= "picture_data";
	ofDirectory picDir(picPath);
	int picDirSize = picDir.listDir();
	picDir.sort();

	info += "loading panoramas from " + picDir.getAbsolutePath() + "\n";
	ofDrawBitmapStringHighlight(info, 800, 300);
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
		info += "Panoramas to load: " + ofToString(all_panorama_filenames.size()) + "\n";
		ofDrawBitmapStringHighlight(info, 800, 300);
		for (int i = 0; i < all_panorama_filenames.size(); i++) {
			vector <ofImage> this_panorama;
			for (int j = 0; j < all_panorama_filenames[i].size(); j++) {
				ofImage picture;
				cout << all_panorama_filenames[i][j].c_str() << endl;
				if (picture.loadImage(all_panorama_filenames[i][j].c_str())) {
					picture.getTextureReference().bind();
					int tex_target = picture.getTextureReference().getTextureData().textureTarget;
					glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(tex_target,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glGenerateMipmap(GL_TEXTURE_2D);
					glTexParameterf( tex_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 10.0f);
					picture.getTextureReference().unbind();
					this_panorama.push_back(picture);
				}
			}
			//only add if all pictures are loaded correctly
			if (this_panorama.size() == 6)
			{
				all_panoramas.push_back(this_panorama);
				cout << "Loaded " << all_panoramas.size() << endl;
				info += "Loaded " + ofToString(all_panoramas.size()) + "\n";
				ofDrawBitmapStringHighlight(info, 800, 300);
			}
		}
	}
	if (all_panoramas.size() >=1) {
		panorama_index = 0;
		return true;
	} else {
		panorama_index = -1;
		return false;
	}
}

//--------------------------------------------------------------
void PPlayerApp::cycleTextures_up(){

	new_panorama_index = panorama_index + 1;
	//to avoid bad accesses
	new_panorama_index = new_panorama_index % all_panoramas.size();
}

//--------------------------------------------------------------
void PPlayerApp::cycleTextures_down() {
	if (panorama_index <= 0)						//to aviod unwanted behavior when going below zero
		new_panorama_index = all_panoramas.size() - 1;
	else
		new_panorama_index = panorama_index - 1;

	//to avoid bad accesses
	new_panorama_index = new_panorama_index % all_panoramas.size();
}

//--------------------------------------------------------------
void PPlayerApp::fade_to(bool &is_black) {
	last_recieved_fading_state = is_black;
}


//--------------------------------------------------------------
bool PPlayerApp::loadSettings(string filename) {
	ofFile settingsfile;
	settingsfile.open(filename);

	if (settingsfile.exists())
	{
		ofXml settings;
		ofBuffer buf = settingsfile.readToBuffer();
		settings.loadFromBuffer(buf.getText());

		if (settings.setTo("settings"))
		{
			/*field of view*/
			if (settings.exists("field_of_view")) {
				fov = settings.getIntValue("field_of_view");
				fov = min(fov, 170);
				fov = max(fov, 10);
			} else
				fov = 65;
			/*rotation scale*/
			if (settings.exists("rotation_scale"))
				rotation_scale = settings.getFloatValue("rotation_scale");
			else
				rotation_scale = 1.0;
			/* fullsreen */
			if (settings.exists("fullscreen"))
				fullscreen = settings.getBoolValue("fullscreen");
			else
				fullscreen = true;
			/*show stats*/
			if (settings.exists("show_stats"))
				show_stats = settings.getBoolValue("show_stats");
			else
				show_stats = true;
			/*blending speed*/
			if (settings.exists("blending_speed"))
				blending_speed = settings.getFloatValue("blending_speed");
			else
				blending_speed = 4.0;
			/* rotation offset */
			if (settings.exists("rotation_offset"))
				rotation_offset = settings.getFloatValue("rotation_offset");
			else
				rotation_offset = 180;
            if (settings.exists("use_shutdown_timer"))
                use_shutdown_timer = settings.getBoolValue("use_shutdown_timer");
            else
                use_shutdown_timer = false;
            if (settings.exists("shutdown_time")) {
                string the_time = settings.getValue("shutdown_time");
                shutdown_time = ofToInt(the_time.substr(0,2)) * 60 + ofToInt(the_time.substr(3,2));
            }

			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------
void PPlayerApp::loadDefaultSettings() {
	fov = 70;
	rotation_scale = 1.0;
	fullscreen = true;
	show_stats = true;
	blending_speed = 8.0;
	use_shutdown_timer = false;
}
