#ifndef _TJCOLORPROPERTY_H
#define _TJCOLORPROPERTY_H

namespace tj {
	namespace shared {
		class ColorPopupWnd;

		/** A color wheel in HSV **/
		class EXPORTED ColorWheel: public virtual Object {
			public:
				ColorWheel();
				virtual ~ColorWheel();
				virtual void SetSize(Pixels x, Pixels y);
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme, Pixels offx, Pixels offy);
				virtual void PaintMarker(graphics::Graphics& g, ref<Theme> theme, Pixels offx, Pixels offy, double h, double s);
				virtual RGBColor GetColorAt(Pixels x, Pixels y, double brightness);
				virtual double GetHueAt(Pixels x, Pixels y);
				virtual double GetSaturationAt(Pixels x, Pixels y);

			protected:
				Pixels _w, _h;
				graphics::Bitmap* _bitmap;
		};
		
		class EXPORTED ColorPopupWnd: public PopupWnd, public Listener<SliderWnd::NotificationChanged> {
			public:
				ColorPopupWnd();
				virtual ~ColorPopupWnd();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual void Layout();
				virtual void Notify(ref<Object> source, const SliderWnd::NotificationChanged& evt);
				virtual void Update();
				virtual RGBColor GetColor() const;
				virtual HSVColor GetHSVColor() const;
				virtual void SetColor(double r, double g, double b);
				virtual void SetColor(const RGBColor& color);
				virtual void SetColor(const HSVColor& hsv);

				struct NotificationChanged {
				};

				Listenable<NotificationChanged> EventChanged;

			protected:
				virtual void OnCreated();
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnSize(const Area& ns);

				const static int KFavouriteColorCount = 10;
				const static Pixels KWheelMargin;
				static RGBColor _favColors[KFavouriteColorCount];

				ref<SliderWnd> _brightness;
				ColorWheel _wheel;
				RGBColor _color;
				float _hue, _sat, _val;
		};

		class EXPORTED ColorChooserWnd: public ChildWnd, public Listener<ColorPopupWnd::NotificationChanged> {
			public:
				ColorChooserWnd(RGBColor* c, RGBColor* tc);
				virtual ~ColorChooserWnd();
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void Notify(ref<Object> source, const ColorPopupWnd::NotificationChanged& data);

			protected:
				RGBColor* _color;
				RGBColor* _tcolor;
				Icon _colorsIcon;
				ref<ColorPopupWnd> _cpw;
		};

		class EXPORTED ColorProperty: public Property {
			public:
				ColorProperty(const std::wstring& name, RGBColor* color, RGBColor* tooColor);
				virtual ~ColorProperty();
				virtual HWND GetWindow();
				virtual std::wstring GetValue();
				virtual HWND Create(HWND parent);
				virtual void Changed();
				virtual void Update();

			protected:
				RGBColor* _color;
				RGBColor* _tcolor;
				ref<ColorChooserWnd> _wnd;
		};
	}
}
#endif