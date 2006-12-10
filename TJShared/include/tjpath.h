#ifndef _TJPATH_H
#define _TJPATH_H

class PropertyGridWnd;

class EXPORTED Crumb: public virtual Object {
	friend class PathWnd;

	public:
		Crumb(std::wstring text, std::wstring icon);
		virtual ~Crumb();
		std::wstring GetText() const;
		std::wstring GetTextTrimmed() const;
		Gdiplus::Bitmap* GetIcon();
		virtual ref< std::vector< ref<Crumb> > > GetChildren();
		virtual ref<Inspectable> GetSubject();
		virtual void SetText(const std::wstring& x);

	protected:
		std::wstring _text;
		Gdiplus::Bitmap* _icon;
		static const int KMaxTextLength = 15;
};

class EXPORTED BasicCrumb: public Crumb {
	public:
		BasicCrumb(std::wstring text, std::wstring icon, ref<Inspectable> subject);
		virtual ~BasicCrumb();
		virtual ref<Inspectable> GetSubject();
		virtual ref< std::vector< ref<Crumb> > > GetChildren();

	protected:
		ref<Inspectable> _subject;
};

class EXPORTED Path: public virtual Object {
	friend class PathWnd;

	public:
		Path();
		virtual ~Path();
		virtual void Add(std::wstring text, std::wstring icon, ref<Inspectable> subject);
		virtual void Add(ref<Crumb> r);
		virtual ref<Crumb> GetHead();

	protected:
		std::vector< ref<Crumb> > _crumbs;
};

class PathWnd: public ChildWnd {
	public:
		PathWnd(HWND parent, PropertyGridWnd* grid);
		virtual ~PathWnd();
		virtual void Update();
		virtual void Paint(Gdiplus::Graphics& g);
		virtual wchar_t GetPreferredHotkey();
		virtual LRESULT Message(UINT msg, WPARAM wp, LPARAM lp);
		virtual void SetPath(ref<Path> p);
		virtual ref<Crumb> GetCrumbAt(int x, int* left=0);

	protected:
		virtual void DoCrumbMenu(ref<Crumb> r, int x);

		const static int KMarginLeft = 3;
		const static int KMarginRight = 12;
		const static int KIconWidth = 16;

		ref<Path> _path;
		PropertyGridWnd* _propertyGrid;
		Gdiplus::Bitmap* _separator;
		ref<Crumb> _over;
};

#endif