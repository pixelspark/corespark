#ifndef _TJ_ANIMATION_H
#define _TJ_ANIMATION_H

namespace tj {
	namespace shared {
		/** This is a 'simple' animation. It helps you determining the value of some animation at a certain point in time,
		but it doesn't have any functionality for calling you back every now and then for repainting, automatic animation
		or retargeting **/
		class EXPORTED Animation {
			public:
				enum Ease {
					EaseLinear = 0,		// f(x) = x
					EaseQuadratic,		// f(x) = x^2
					EaseCubic,			// f(x) = x^3
					EasePulse,			// f(x < 0.5) = x*2, f(x > 0.5) = 2*(1-x)
					EaseBlink,			// f(x) = fmod(x,r) == 0
				};

				Animation();
				virtual ~Animation();
				virtual void Start(const Time& length, bool reverse = false, Ease ease = EaseLinear);
				virtual void Start();
				virtual void Stop();
				virtual float GetProgress() const;
				virtual float GetFraction() const;
				virtual float GetFraction(Ease e) const;
				virtual bool IsAnimating() const;
				virtual bool IsReversed() const;

				virtual void SetReversed(bool r);
				virtual void SetEase(Ease e);
				virtual void SetLength(const Time& l);

				static void SetAnimationsEnabled(bool t);
				static bool IsAnimationsEnabled();

			private:
				static bool _animationsEnabled;

				Timestamp _startTime;
				Time _length;
				Ease _ease;
				bool _reversed;
		};

		/** The more advanced animation framework starts here. An 'Animatable' object is an object that holds 
		Animated values. The method OnAnimationStep is called whenever an animation is being executed and it is necessary
		to repaint. **/
		class Animated;
		class AnimationBlock;

		namespace intern {
			class AnimationThread;
		}

		class EXPORTED Animatable: public virtual Object {
		  public:
			virtual ~Animatable();
			virtual void OnAnimationStep(const Animated& member) = 0; // Will always be called from a separate animation thread
		};

		/** Identifies an animated value. This class should *ALWAYS* be created as a member of a class that implements
		Animatable (and the 'parent' pointer in the constructor should always point to that class) **/
		class EXPORTED Animated {
			friend class AnimationTarget;

			public:
				Animated(Animatable* parent, const double& initValue);
				~Animated();
				double GetValue() const;
				void SetValue(double d);
				operator double() const;
			    
			  protected:
				void SetAnimatedValue(double v);
			    
				volatile double _value;
				Animatable* _parent;
		};

		class EXPORTED AnimationTarget {
			public:
				AnimationTarget(ref<Animatable> am, Animated* value);
				~AnimationTarget();
				bool operator==(const AnimationTarget& o) const;
				bool operator<(const AnimationTarget& o) const;
				void SetAnimatedValue(double r) const;
		    
			protected:
				weak<Animatable> _am;
				Animated* _value;
		};

		class EXPORTED AnimationTargetValue {
			friend class AnimationManager;
			friend class tj::shared::intern::AnimationThread;

			public:
				AnimationTargetValue(double t = 0.0, double tPerSecond = 0.0);
				~AnimationTargetValue();

			protected:
				double _value;
				double _speed;
		};

		class EXPORTED AnimationManager: public Singleton<AnimationManager> {
			friend class AnimationBlock;
			friend class Animated;
			friend class tj::shared::intern::AnimationThread;
			friend class Singleton<AnimationManager>;

			public:
				virtual ~AnimationManager();
				virtual bool IsAnimationEnabled() const;
				virtual void SetAnimationEnabled(bool t);
				virtual float GetDesiredFrameRate() const;

			protected:
				AnimationManager();
				virtual AnimationBlock* GetCurrentAnimationBlock();
				virtual AnimationBlock* PushAnimationBlock(AnimationBlock* ab);
				virtual void PopAnimationBlock(AnimationBlock* prev);
				virtual void CommitBlock(const AnimationBlock& ab);

			private:
				void EnsureAnimationThreadStarted();

				CriticalSection _targetsLock;
				Event _animationsQueuedEvent;
				std::map< AnimationTarget, std::map<Timestamp, AnimationTargetValue> > _targets;
				ThreadLocal _currentBlock;
				ref<Thread> _thread;
				volatile bool _enabled;
		};

		class EXPORTED AnimationBlock {
			friend class AnimationManager;
			friend class Animated;

			public:
				AnimationBlock(const Time& duration = 1);
				~AnimationBlock();
				void Commit();

			protected:
				void AddTarget(ref<Animatable> am, Animated* val, double currentValue, double futureValue);

				AnimationBlock* _prev;
				Time _duration;
				std::map<AnimationTarget, std::pair<double, double> > _values;
		};
	}
}

#endif