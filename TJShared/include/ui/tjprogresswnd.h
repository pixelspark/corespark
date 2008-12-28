#ifndef _TJPROGRESSWND_H
#define _TJPROGRESSWND_H

namespace tj {
	namespace shared {
		class EXPORTED ProgressWnd: public ChildWnd {
			public:
				ProgressWnd();
				virtual ~ProgressWnd();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				void SetIndeterminate(bool t);
				void SetValue(float x);
				void Update();

			protected:
				virtual void OnTimer(unsigned int id);

				bool _indeterminate;
				float _value;
		};
	}
}

#endif