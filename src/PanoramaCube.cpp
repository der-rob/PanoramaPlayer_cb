#include "PanoramaCube.h"


PanoramaCube::PanoramaCube(void)
{
	x = y = z = 16000;
	x = -width/2;
	y = -height/2;
	z = -length/2;
	center.set(0,0,0);
}


PanoramaCube::~PanoramaCube(void)
{
}

PanoramaCube::PanoramaCube(int _width, ofVec3f _center) {
	width = _width;
	height = _width;
	length = _width;

	x = -width/2;
	y = -height/2;
	z = -length/2;

	center = ofVec3f(_center);


}

void PanoramaCube::render(vector < ofImage> & _panorama, float _rotation) {
	ofPushMatrix();
		ofTranslate(center.x, center.y,0);
		ofRotate(_rotation, 0, 1, 0);

		// Draw Front side
		_panorama[0].getTextureReference().bind();
		

		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z+length);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y+height, z+length);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y+height, z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y, z+length);
		glEnd();
		_panorama[0].getTextureReference().unbind();

		// Draw Back side
		_panorama[1].getTextureReference().bind();
		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y, z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y+height, z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y+height, z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, z);
		glEnd();
		_panorama[1].getTextureReference().unbind();

		// Draw up side
		_panorama[2].getTextureReference().bind();
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y+height, z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y+height, z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y+height, z+length);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y+height, z);
		glEnd();
		_panorama[2].getTextureReference().unbind();

		// Draw down side
		_panorama[3].getTextureReference().bind();
		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z+length);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y, z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y, z);
		glEnd();
		_panorama[4].getTextureReference().unbind();

		// Draw Left side
		_panorama[4].getTextureReference().bind();
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y, z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y, z+length);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y+height, z+length);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y+height, z);
		glEnd();
		_panorama[4].getTextureReference().unbind();

		// Draw Right side
		_panorama[5].getTextureReference().bind();
		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y+height, z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y+height, z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z);
		glEnd();
		_panorama[5].getTextureReference().unbind();

		ofPopMatrix();

}
