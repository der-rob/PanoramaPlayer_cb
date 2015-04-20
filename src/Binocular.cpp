#include "Binocular.h"


Binocular::Binocular(string _vert, string _frag)
{
	shader_loaded = shader.load(_vert,_frag);
	alpha = 0.0;
	use_binocular = false;
}
Binocular::Binocular() {
	shader_loaded = false;
}

Binocular::~Binocular(void)
{
	shader.unload();
}

void Binocular::render() {

	ofPushMatrix();
	ofMatrixMode(OF_MATRIX_PROJECTION);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//shader
	//draws the binocculars mask
	shader.begin();
	shader.setUniform1f("alpha", alpha);
	shader.setUniform1i("use_binocular", use_binocular);
	aspect_ratio = (float)ofGetWidth() / (float)ofGetHeight();
	shader.setUniform1f("aspect_ratio", aspect_ratio);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(ofGetWidth(), 0);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(ofGetWidth(), ofGetHeight());
	glTexCoord2f(0.0f, 1.0f); glVertex2f(0, ofGetHeight());
	glEnd();
	shader.end();
	ofPopMatrix();
}

void Binocular::set_alpha(float _alpha){
	alpha = _alpha;
}

void Binocular::set_use_binocular(bool use_it) {
    use_binocular = use_it;
}
