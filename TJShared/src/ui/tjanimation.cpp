#include "../../include/ui/tjui.h" 
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
		}
	}
}

using namespace tj::shared::intern;

/** Animatable **/
Animatable::~Animatable() {
}

/** Animated **/
Animated::Animated(Animatable* parent, const double& initValue): _parent(parent), _value(initValue) {
}

Animated::~Animated() {
}

double Animated::GetValue() const {
	return _value;
}

void Animated::SetValue(double d) {
	AnimationBlock* ab = AnimationManager::Instance()->GetCurrentAnimationBlock();
	if(ab!=0) {
		ab->AddTarget(ref<Animatable>(_parent), this, _value, d);
	}
}

void Animated::SetAnimatedValue(double v) {
	_value = v;
}

Animated::operator double() const {
	return _value;
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

void AnimationTarget::SetAnimatedValue(double r) const {
	ref<Animatable> am = _am;
	if(am) {
		_value->SetAnimatedValue(r);
		am->OnAnimationStep(*_value);
	}
}

/** AnimationTargetValue **/
AnimationTargetValue::AnimationTargetValue(double val, double speed): _value(val), _speed(speed) {
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
			}
			++it;
		}
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
			AnimationTargetValue targetValue(it->second.second, (it->second.second - it->second.first) / (double(ab._duration.ToInt())/1000.0));

			std::map< AnimationTarget, std::map<Timestamp, AnimationTargetValue> >::iterator mit = _targets.find(it->first);
			if(mit==_targets.end()) {
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
				if(timedValues.size() < 1) {
					it = am->_targets.erase(it);
				}
				else {
					const AnimationTarget& target = it->first;
					std::map<Timestamp, AnimationTargetValue>::iterator tit = timedValues.begin();
					while(tit != timedValues.end()) {
						const AnimationTargetValue& targetValue = tit->second;

						if(tit->first.IsEarlierThan(currentTime)) {
							target.SetAnimatedValue(targetValue._value);
							tit = timedValues.erase(tit);
						}
						else {
							double diffMS = tit->first.Difference(currentTime).ToMilliSeconds();
							double value = targetValue._value - (targetValue._speed * (diffMS/1000.0));
							target.SetAnimatedValue(value);
							++tit;
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
AnimationBlock::AnimationBlock(const Time& duration): _duration(duration) {
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