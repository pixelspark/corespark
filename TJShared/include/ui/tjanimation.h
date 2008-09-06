#ifndef _TJ_ANIMATION_H
#define _TJ_ANIMATION_H

namespace tj {
	namespace shared {
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
	}
}

#endif