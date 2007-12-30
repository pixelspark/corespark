#ifndef _TJCOLORPROPERTY_H
#define _TJCOLORPROPERTY_H

namespace tj {
	namespace shared {
		class ColorPopupWnd;

		class EXPORTED ColorChooserWnd: public ChildWnd, public Listener {
			public:
				ColorChooserWnd(unsigned char* red, unsigned char* green, unsigned char* blue, unsigned char* tred, unsigned char* tgreen, unsigned char* tblue);
				virtual ~ColorChooserWnd();
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void Notify(Wnd* source, Notification n);

			protected:
				unsigned char* _red;
				unsigned char* _green;
				unsigned char* _blue;
				unsigned char* _tred;
				unsigned char* _tgreen;
				unsigned char* _tblue;
				Icon _colorsIcon;
				ref<ColorPopupWnd> _cpw;
		};

		/** A color wheel in HSV **/
		class EXPORTED ColorWheel: public virtual Object {
			public:
				ColorWheel();
				virtual ~ColorWheel();
				virtual void SetSize(Pixels x, Pixels y);
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme, Pixels offx, Pixels offy);
				virtual void PaintMarker(Gdiplus::Graphics& g, ref<Theme> theme, Pixels offx, Pixels offy, double h, double s);
				virtual Gdiplus::Color GetColorAt(Pixels x, Pixels y, double brightness);
				virtual double GetHueAt(Pixels x, Pixels y);
				virtual double GetSaturationAt(Pixels x, Pixels y);

			protected:
				Pixels _w, _h;
				Gdiplus::Bitmap* _bitmap;
		};
		
		class ColorPopupWnd: public PopupWnd, public Listener {
			public:
				ColorPopupWnd();
				virtual ~ColorPopupWnd();
				virtual void Paint(Gdiplus::Graphics& g, ref<Theme> theme);
				virtual void Layout();
				virtual void Notify(Wnd* source, Notification evt);
				virtual void Update();
				virtual void SetListener(ref<Listener> ls);
				virtual Gdiplus::Color GetColor();
				virtual void SetColor(double r, double g, double b);

			protected:
				virtual void OnCreated();
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnSize(const Area& ns);

				ref<SliderWnd> _brightness;
				weak<Listener> _myListener;
				ColorWheel _wheel;
				Gdiplus::Color _color;
				float _hue, _sat, _val;
				const static Pixels KWheelMargin;
		};

		class EXPORTED ColorProperty: public Property {
			public:
				ColorProperty(std::wstring name,unsigned char* red, unsigned char* green, unsigned char* blue,unsigned char* tred, unsigned char* tgreen, unsigned char* tblue);
				virtual ~ColorProperty();
				virtual HWND GetWindow();
				virtual std::wstring GetValue();
				virtual HWND Create(HWND parent);
				virtual void Changed();
				virtual void Update();

			protected:
				unsigned char* _red;
				unsigned char* _green;
				unsigned char* _blue;
				unsigned char* _tRed;
				unsigned char* _tBlue;
				unsigned char* _tGreen;
				ref<ColorChooserWnd> _wnd;
		};
	}
}
#endif