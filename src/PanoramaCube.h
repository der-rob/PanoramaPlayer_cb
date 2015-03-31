#pragma once
#include "ofMain.h"

class PanoramaCube
{
public:
	PanoramaCube(void);
	PanoramaCube(int width, ofVec3f center);
	~PanoramaCube(void);

	void render(vector <ofImage> & _panorama, float _rotation);

private:
	int width;
	int height;
	int length;
	int x,y,z;
	ofVec3f center;
};

