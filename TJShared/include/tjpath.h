#ifndef _TJPATH_H
#define _TJPATH_H

class EXPORTED Path: public virtual Object {
	friend class PathWnd;

	public:
		struct EXPORTED Crumb {
			friend class PathWnd;

			Crumb(std::wstring text, std::wstring icon, ref<Inspectable> subject);
			virtual ~Crumb();

			std::wstring _text;
			Gdiplus::Bitmap* _icon;
			ref<Inspectable> _subject;
		};

		Path();
		virtual ~Path();
		virtual void Add(std::wstring text, std::wstring icon, ref<Inspectable> subject);
		virtual void Add(ref<Path::Crumb> r);

	protected:
		std::vector< ref<Path::Crumb> > _crumbs;
};

class PathWnd: public ChildWnd {
	public:
		PathWnd(HWND parent);
		virtual ~PathWnd();
		virtual void Update();
		virtual void Paint(Gdiplus::Graphics& g);
		virtual wchar_t GetPreferredHotkey();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual void SetPath(ref<Path> p);
		virtual ref<Path::Crumb> GetCrumbAt(int x, int* left=0);

	protected:
		virtual void DoCrumbMenu(ref<Path::Crumb> r, int x);

		const static int KMarginLeft = 3;
		const static int KMarginRight = 12;
		const static int KIconWidth = 16;

		ref<Path> _path;
		Gdiplus::Bitmap* _separator;
		ref<Path::Crumb> _over;
};

#endif