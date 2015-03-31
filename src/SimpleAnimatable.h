#include <iostream>
#include <math.h>
#define NULL 0
#define PI 3.14159265358979323846
#pragma once

struct ANIMATION
{
	float start;
	float end;
	float t;
	float v;
};

enum ENUM_ANIMATION_STATES { DARK, BRIGHT, GO_DARK, GO_BRIGHT };

class SimpleAnimatable
{
public:
	SimpleAnimatable(void);
	~SimpleAnimatable(void);
	
private:
	float m_value;
	ANIMATION *m_ani;
	ENUM_ANIMATION_STATES m_animation_state;
	
	void setAnimation(ANIMATION *_ani);
	float doubleExponentialSigmoid (float x, float a);
public:
	bool updateAnimation(float dt);
	void createAnimation(float start, float end, float speed, ENUM_ANIMATION_STATES anim_state);
	ANIMATION* getAnimation() {return m_ani;}
	float getValue() {return m_value;}
	ENUM_ANIMATION_STATES getAnimationState() {return m_animation_state;}
};

