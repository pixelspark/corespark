#include "../../include/ui/tjui.h" 
using namespace tj::shared;

bool Animation::_animationsEnabled = true;

Animation::Animation(): _startTime(false), _length(0), _ease(EaseLinear), _reversed(false) {
}

Animation::~Animation() {
}

void Animation::Start() {
	_startTime.Now();
}

void Animation::Start(const Time& length, bool rev, Ease ease) {
	_reversed = rev;
	_ease = ease;
	_length = length;
	_startTime.Now();
}

void Animation::Stop() {
	_length = 0;
}

float Animation::GetProgress() const {
	if(_animationsEnabled) {
		Timestamp now(true);
		float p = float((now.Difference(_startTime).ToMilliSeconds())/(long double)(_length.ToInt()));
		if(p>1.0f) p = 1.0f;
		if(p<0.0f) p = 0.0f;
		return _reversed ? (1-p) : p;
	}
	else {
		return _reversed ? 0.0f : 1.0f;
	}
}

float Animation::GetFraction() const {
	return GetFraction(_ease);
}

float Animation::GetFraction(Ease ease) const {
	float p = GetProgress();

	switch(ease) {
		case EaseQuadratic:
			return p*p;

		case EaseCubic:
			return p*p*p;

		case EasePulse: {
			if(p<=0.5f) {
				return 2*p;
			}			
			else {
				return 1 - 2*(p-0.5f);
			}
		}

		case EaseBlink:
			return fmod(p, 0.1f) > 0.05f;

		case EaseLinear:
		default:
			return p;
	}
}

bool Animation::IsAnimating() const {
	if(!_animationsEnabled) {
		return false;
	}

	Timestamp now(true);
	return now.Difference(_startTime).ToMilliSeconds() < _length.ToInt();
}

void Animation::SetReversed(bool e) {
	_reversed = e;
}

bool Animation::IsReversed() const {
	return _reversed;
}

void Animation::SetEase(Ease e) {
	_ease = e;
}

void Animation::SetLength(const Time& l) {
	_length = l;
}

void Animation::SetAnimationsEnabled(bool t) {
	_animationsEnabled = t;
}

bool Animation::IsAnimationsEnabled() {
	return _animationsEnabled;
}