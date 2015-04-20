#pragma once
#include "ofMain.h"

class Binocular
{
public:
	Binocular();
	Binocular(string _vert, string _frag);
	~Binocular(void);
private:
	ofShader shader;
	bool shader_loaded;
	float alpha;
	float aspect_ratio;
	bool use_binocular;

public:
	void render();
	void set_alpha(float _alpha);
	void set_use_binocular(bool use_it);
};

