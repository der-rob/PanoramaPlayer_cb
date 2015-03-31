#include "SimpleAnimatable.h"


SimpleAnimatable::SimpleAnimatable(void)
{
	m_value = 0.0;
	m_ani = NULL;
	m_animation_state = BRIGHT;
}


SimpleAnimatable::~SimpleAnimatable(void)
{
}

void SimpleAnimatable::createAnimation(float start, float end, float speed, ENUM_ANIMATION_STATES anim_state) {
	ANIMATION *ani = new ANIMATION();
	ani->start = start;
	ani->end = end;
	ani->t = 0.0;
	ani->v = speed;
	setAnimation(ani);
	m_animation_state = anim_state;
}

void SimpleAnimatable::setAnimation(ANIMATION *_ani) {
	m_ani = _ani;
}

bool SimpleAnimatable::updateAnimation(float dt) {
	//trigger gets true when aniamtion reaches loop back point
	bool trigger = false;
	if (m_ani->t <=1)
	{
		float distance = m_ani->end - m_ani->start;
		//float valueToAdd = m_ani->t*distance;
		float valueToAdd = doubleExponentialSigmoid(m_ani->t,0.5)*distance;
		m_value = m_ani->start + valueToAdd;
		m_ani->t += dt*m_ani->v;
	}
	else
	{
		if (m_animation_state == GO_DARK)
		{
			trigger = true;
			m_animation_state = DARK;
		}
		else if (m_animation_state == GO_BRIGHT)
		{
			m_animation_state = BRIGHT;
		}
		m_ani = NULL;
	}
	return trigger;
}

float SimpleAnimatable::doubleExponentialSigmoid (float x, float a){

  float epsilon = 0.00001;
  float min_param_a = 0.0 + epsilon;
  float max_param_a = 1.0 - epsilon;
  a = std::min(max_param_a, std::max(min_param_a, a));
  a = 1.0-a; // for sensible results
  
  float y = 0;
  if (x<=0.5){
    y = (pow(2.0*x, 1.0/a))/2.0;
  } else {
    y = 1.0 - (pow(2.0*(1.0-x), 1.0/a))/2.0;
  }
  return y;
}
