#ifndef _TJSPLITTER_H
#define _TJSPLITTER_H

#pragma warning(push)
#pragma warning(disable: 4251)

enum Orientation {
	OrientationHorizontal=0,
	OrientationVertical,
};

class EXPORTED SplitterWnd: public ChildWnd {
	friend class ChildEnumerator;

	public:
		SplitterWnd(HWND parent, ref<Wnd> a, ref<Wnd> b, Orientation o);
		virtual ~SplitterWnd();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual void Layout();
		virtual void Update();
		void SetRatio(float f);
		virtual bool IsSplitter();
		virtual void Paint(Gdiplus::Graphics& g);
		void Collapse();
		void Expand();

	protected:
		enum {barHeight = 4 };
		const static float snapMargin;

		ref<Wnd> _a;
		ref<Wnd> _b;
		float _ratio;
		float _defaultRatio;
		bool _dragging;
		Orientation _orientation;
};

#pragma warning(pop)
#endif