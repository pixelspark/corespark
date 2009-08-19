#include "../include/tjsharedui.h"
using namespace tj::shared;

namespace tj {
	namespace shared {
		namespace intern {
			class AnimationThread: public Thread {
				public:
					AnimationThread(ref<AnimationManager> am);
					virtual ~AnimationThread();
					virtual void Run();

				private:
					weak<AnimationManager> _am;
			};

			double AnimationEaseLinear(double in) {
				return in;
			}

			double AnimationEaseQuadratic(double in) {
				return in*in;
			}

			double AnimationEaseCubic(double in) {
				return in*in*in;
			}

			double AnimationEasePulse(double in) {
				if(in<0.5) {
					return in * 2.0;
				}
				else {
					return 2.0 * (1.0-in);
				}
			}

			double AnimationEaseOvershoot(double in) {
				return sin(in * 0.7 * 3.14159) * 1.236;
			}

			double AnimationEaseBlink(double in) {
				return (fmod((float)in, 0.1f) > 0.05f) ? 1.0 : 0.0;
			}
		}
	}
}

using namespace tj::shared::intern;

/** Animation eases **/
AnimationEase Animation::GetEase(Animation::Ease f) {
	switch(f) {
		case Animation::EaseLinear:
			return intern::AnimationEaseLinear;

		case Animation::EaseQuadratic:
			return intern::AnimationEaseQuadratic;

		case Animation::EaseCubic:
			return intern::AnimationEaseCubic;

		case Animation::EasePulse:
			return intern::AnimationEasePulse;

		case Animation::EaseOvershoot:
			return intern::AnimationEaseOvershoot;

		case Animation::EaseBlink:
			return intern::AnimationEaseBlink;
	}

	return 0;
}

/** Animatable **/
Animatable::~Animatable() {
}

/** Animated **/
Animated::Animated(Animatable* parent, const double& initValue): _parent(parent), _isAnimating(false), _value(initValue), _futureValue(initValue) {
}

Animated::~Animated() {
}

double Animated::GetValue() const {
	return _value;
}

double Animated::GetFutureValue() const {
	return _futureValue;
}

void Animated::SetValue(double d) {
	_futureValue = d;
	try {
		AnimationBlock* ab = AnimationManager::Instance()->GetCurrentAnimationBlock();
		if(ab!=0) {
			ab->AddTarget(ref<Animatable>(_parent), this, _value, d);
		}
		else {
			_value = d;
		}
	}
	catch(const Exception&) {
		// A reference error can occur if SetValue is called from within the constructor of the Animatable object.
		// At that point, the Animatable object cannot be referenced yet.
		_value = d;
	}
}

void Animated::SetAnimatedValue(double v) {
	_value = v;
}

Animated::operator double() const {
	return _value;
}

const double& Animated::operator=(const double& o) {
	SetValue(o);
	return o;
}

bool Animated::IsAnimating() const {
	return _isAnimating;
}

void Animated::SetAnimating(bool t) {
	_isAnimating = t;
}

const double& Animated::operator+=(const double& o) {
	SetValue(_futureValue + o);	
	return o;
}

const double& Animated::operator-=(const double& o) {
	SetValue(_futureValue - o);	
	return o;
}

/** AnimationTarget **/
AnimationTarget::AnimationTarget(ref<Animatable> am, Animated* value): _am(am), _value(value) {
}

AnimationTarget::~AnimationTarget() {
}

bool AnimationTarget::operator<(const AnimationTarget& o) const {
	return _value < o._value;
}

bool AnimationTarget::operator==(const AnimationTarget& o) const {
	return o._value == _value;
}

void AnimationTarget::SetAnimating(bool t) const {
	ref<Animatable> am = _am;
	if(am) {
		_value->SetAnimating(t);
	}
}

void AnimationTarget::SetAnimatedValue(double r) const {
	ref<Animatable> am = _am;
	if(am) {
		_value->SetAnimatedValue(r);
		am->OnAnimationStep(*_value);
	}
}

/** AnimationTargetValue **/
AnimationTargetValue::AnimationTargetValue(double val, double speed, AnimationEase ease, const Time& duration): _value(val), _speed(speed), _ease(ease), _duration(duration) {
}

AnimationTargetValue::~AnimationTargetValue() {
}

/** AnimationManager **/
AnimationManager::AnimationManager(): _enabled(true) {
}

AnimationManager::~AnimationManager() {
}

float AnimationManager::GetDesiredFrameRate() const {
	// TODO make this some setting (maybe use OnSettingsChange etc. like Wnd does?)
	return 25.0f;
}

bool AnimationManager::IsAnimationEnabled() const {
	return _enabled;
}

void AnimationManager::SetAnimationEnabled(bool t) {
	ThreadLock lock(&_targetsLock);
      
	if(!t && _enabled) {
		std::map< AnimationTarget, std::map<Timestamp, AnimationTargetValue> >::iterator it = _targets.begin();
		while(it!=_targets.end()) {
			const AnimationTarget& target = it->first;
			std::map<Timestamp, AnimationTargetValue>::reverse_iterator vit = it->second.rbegin();
			if(vit!=it->second.rend()) {
				const AnimationTargetValue& targetValue = vit->second;
				target.SetAnimatedValue(targetValue._value);
				target.SetAnimating(false);
			}
			++it;
		}
		_targets.clear();
	}

	_enabled = t;
}

AnimationBlock* AnimationManager::GetCurrentAnimationBlock() {
	return reinterpret_cast<AnimationBlock*>(_currentBlock.GetValue());
}

AnimationBlock* AnimationManager::PushAnimationBlock(AnimationBlock* ab) {
	AnimationBlock* prev = GetCurrentAnimationBlock();
	_currentBlock.SetValue(reinterpret_cast<void*>(ab));
	return prev;
}

void AnimationManager::PopAnimationBlock(AnimationBlock* prev) {
	_currentBlock.SetValue(reinterpret_cast<void*>(prev));
}

void AnimationManager::CommitBlock(const AnimationBlock& ab) {
	if(ab._values.size()<1) {
		return;
	}

	ThreadLock lock(&_targetsLock);
	if(!_enabled) {
		std::map<AnimationTarget, std::pair<double, double> >::const_iterator it = ab._values.begin();
		while(it!=ab._values.end()) {
			const AnimationTarget& target = it->first;
			target.SetAnimatedValue(it->second.second);
			++it;
		}
	}
	else {
		Timestamp endTime = Timestamp(true).Increment(ab._duration);
		std::map<AnimationTarget, std::pair<double, double> >::const_iterator it = ab._values.begin();
		while(it!=ab._values.end()) {
			AnimationTargetValue targetValue(it->second.second, (it->second.second - it->second.first) / (double(ab._duration.ToInt())/1000.0), ab._ease, ab._duration.ToInt());

			std::map< AnimationTarget, std::map<Timestamp, AnimationTargetValue> >::iterator mit = _targets.find(it->first);
			if(mit==_targets.end()) {
				it->first.SetAnimating(true);
				_targets[it->first] = std::map<Timestamp, AnimationTargetValue>();
				mit = _targets.find(it->first);
			}

			mit->second[endTime] = targetValue;
			++it;
		}
		EnsureAnimationThreadStarted();
		_animationsQueuedEvent.Signal();
	}
}

void AnimationManager::EnsureAnimationThreadStarted() {
	if(!_thread) {
		ThreadLock lock(&_targetsLock);
		if(!_thread) {
			_thread = GC::Hold(new intern::AnimationThread(this));
			_thread->Start();
		}
	}
}

/** AnimationThread **/
AnimationThread::AnimationThread(ref<AnimationManager> am): _am(am) {
}

AnimationThread::~AnimationThread() {
}

void AnimationThread::Run() {
	Log::Write(L"TJShared/Animation", L"AnimationThread started");
	while(true) {
		ref<AnimationManager> am = _am;
		if(!am) {
			return;
		}

		float fps = am->GetDesiredFrameRate();
		bool haveWork = false;

		{
			ThreadLock lock(&(am->_targetsLock));
			am->_animationsQueuedEvent.Reset();
			Timestamp currentTime(true);

			std::map< AnimationTarget, std::map<Timestamp, AnimationTargetValue> >::iterator it = am->_targets.begin();
			while(it!=am->_targets.end()) {
				std::map<Timestamp, AnimationTargetValue>& timedValues = it->second;
				const AnimationTarget& target = it->first;
				if(timedValues.size() < 1) {
					target.SetAnimating(false);
					it = am->_targets.erase(it);
				}
				else {
					std::map<Timestamp, AnimationTargetValue>::iterator tit = timedValues.begin();
					while(tit != timedValues.end()) {
						const AnimationTargetValue& targetValue = tit->second;

						try {
							if(tit->first.IsEarlierThan(currentTime)) {
								target.SetAnimatedValue(targetValue._value);
								tit = timedValues.erase(tit);
							}
							else {
								double timeToEnd = tit->first.Difference(currentTime).ToMilliSeconds();
								double fraction = 1.0 - (timeToEnd / double(targetValue._duration.ToInt()));
								if(targetValue._ease != 0) {
									fraction = targetValue._ease(fraction);
								}
								double value = targetValue._value - (1.0-fraction) * double(targetValue._duration.ToInt())/1000.0 * targetValue._speed;
								target.SetAnimatedValue(value);
								++tit;
							}
						}
						catch(const Exception& e) {
							// When exceptions occur in the OnAnimationStep method of an Animatable object, they should not crash
							// the animation thread.
							Log::Write(L"TJShared/AnimationThread", L"Exception occurred in AnimationThread, probably in some OnAnimationStep method: "+e.GetMsg());
						}
						catch(...) {
							Log::Write(L"TJShared/AnimationThread", L"An unknown exception has occurred in the AnimationThread (probably within some OnAnimationStep method).");
						}
					}
					++it;
				}
			}

			if(am->_targets.size() > 0) {
				haveWork = true;
			}
		}

		if(haveWork) {
			Thread::Sleep(1.0f/fps);
		}
		else {
			Wait::For(am->_animationsQueuedEvent);
		}
	}
}

/** AnimationBlock **/
AnimationBlock::AnimationBlock(const Time& duration): _duration(duration), _ease(0) {
	_prev = AnimationManager::Instance()->PushAnimationBlock(this);
}

AnimationBlock::~AnimationBlock() {
	Commit();
	AnimationManager::Instance()->PopAnimationBlock(_prev);
}

void AnimationBlock::Commit() {
	AnimationManager::Instance()->CommitBlock(*this);
	_values.clear();
}

void AnimationBlock::SetEase(AnimationEase e) {
	_ease = e;
}

void AnimationBlock::AddTarget(ref<Animatable> am, Animated* val, double currentValue, double futureValue) {
	_values[AnimationTarget(am,val)] = std::pair<double,double>(currentValue, futureValue);
}

/** Animation **/
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
	AnimationEase easeFunction = Animation::GetEase(ease);
	if(easeFunction!=0) {
		return (float)easeFunction((double)p);
	}

	return p;
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

/** AnimatedArea **/
AnimatedArea::AnimatedArea(Animatable* parent): BasicRectangle<double, Animated>(Animated(parent, 0.0), Animated(parent,0.0), Animated(parent,0.0), Animated(parent,0.0)) {
}

AnimatedArea::~AnimatedArea() {
}