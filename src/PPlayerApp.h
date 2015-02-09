#pragma once

#include "ofMain.h"

class PPlayerApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

private:
    int h, width, height, length;
    float x, y, z;
	bool vsync;
	ofVec3f center;

	//ofImage **current_panorama;
	vector<vector <ofImage> > all_panoramas;

	int texture_index;

	void scanTextureFolder();
	void cycleTextures();

    ofEasyCam camera;
    float fov;

    float rotation;
    float rotation_step;

	ofArduino arduino;
	bool bArduinoSetup;
	void setupArduino(const int & version);
    void digitalPinChanged(const int & pinNum);
    void analogPinChanged(const int & pinNum);
	void updateArduino();

    string buttonState;
    string potValue;

	//potentiometer smoothing
	static const int numReadings = 5;
	int readings[numReadings];
	int index, total, average;

	int readButtonDebpunced(int pin, long int debounceDelay, ofArduino &ard);

};
