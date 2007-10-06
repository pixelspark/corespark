#ifndef _TJPROGRESSWND_H
#define _TJPROGRESSWND_H

namespace tj {
	namespace shared {
		class EXPORTED ProgressWnd: public ChildWnd {
			public:
				ProgressWnd();
				virtual ~ProgressWnd();
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);
				virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
				void SetIndeterminate(bool t);
				void SetValue(float x);
				void Update();

			protected:
				bool _indeterminate;
				float _value;
		};
	}
}

#endif