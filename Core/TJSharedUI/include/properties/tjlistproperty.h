/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJLISTPROPERTY_H
#define _TJLISTPROPERTY_H

namespace tj {
	namespace shared {
		template<typename T> class GenericListProperty: public Property {
			public:
				struct Item {
					Item(): _separator(false) {
					}

					String _name;
					T _value;
					bool _separator;

					bool operator <(const Item& b) const {
						return _name < b._name;
					}
				};

				class PropertyWnd: public ChildWnd {
					public:
						PropertyWnd(GenericListProperty<T>& prop, const String& icon): _icon(icon), _prop(prop), _arrowIcon(Icons::GetIconPath(Icons::IconDownArrow)) {
							SetWantMouseLeave(true);
						}

						virtual ~PropertyWnd() {
						}

						virtual void Paint(graphics::Graphics& g, strong<Theme> theme) {
							// Draw background
							Area rc = GetClientArea();
							graphics::SolidBrush back(theme->GetColor(Theme::ColorBackground));
							g.FillRectangle(&back, rc);
							theme->DrawInsetRectangleLight(g, rc);
							
							if(HasFocus()) {
								theme->DrawToolbarBackground(g, 0.0f, 0.0f, float(rc.GetWidth()), float(rc.GetHeight()));
							}

							graphics::SolidBrush borderBrush(theme->GetColor(Theme::ColorActiveStart));
							Area borderArea = rc;
							borderArea.Narrow(0,0,1,1);
							graphics::Pen borderPen(&borderBrush, 1.0f);
							g.DrawRectangle(&borderPen, borderArea);
							
							Area buttonArea(rc.GetRight()-16, rc.GetTop(), 16,rc.GetHeight());

							graphics::LinearGradientBrush buttonBr(graphics::PointF(0.0f, float(rc.GetTop()-1)), graphics::PointF(0.0f, float(rc.GetBottom()+1)), theme->GetColor(Theme::ColorActiveStart), theme->GetColor(Theme::ColorActiveEnd));
							g.FillRectangle(&buttonBr, buttonArea);

							if(!IsMouseOver()) {
								buttonArea.Narrow(1,1,1,1);
								graphics::SolidBrush disabledBr(theme->GetColor(Theme::ColorDisabledOverlay));
								g.FillRectangle(&disabledBr, buttonArea);
							}

							// Draw icon to the right
							Area iconArea(rc.GetRight()-16, rc.GetTop(), 16, 16);
							g.DrawImage(_arrowIcon, iconArea);

							// Text parameters
							graphics::SolidBrush tbr(theme->GetColor(Theme::ColorText));
							Area text = rc;
							text.Narrow(2,2,16+2,0);
							if(_icon.IsLoaded()) {
								text.Narrow(18,0,0,0);
								Area iconArea = rc;
								iconArea.SetWidth(16);
								iconArea.SetHeight(16);
								iconArea.Translate(2,0);
								_icon.Paint(g, iconArea);
							}
							graphics::StringFormat sf;
							sf.SetTrimming(graphics::StringTrimmingEllipsisPath);
							sf.SetFormatFlags(graphics::StringFormatFlagsLineLimit);

							// Fetch text
							T value = *(_prop._value);
							String displayText = L"";
							typename std::vector<Item>::const_iterator it = _prop._options.begin();
							while(it!=_prop._options.end()) {
								const Item& item = *it;
								if(item._value==value) {
									displayText = item._name;
									break;
								}
								++it;
							}

							// Draw text
							g.DrawString(displayText.c_str(), (int)displayText.length(), theme->GetGUIFont(), text, &sf, &tbr);
						}

						virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y) {
							if(ev==MouseEventMove) {
								SetWantMouseLeave(true);
								Repaint();
							}
							else if(ev==MouseEventLeave) {
								Repaint();
							}
							else if(ev==MouseEventLDown) {
								Focus();
								OnContextMenu(x,y);
							}
							ChildWnd::OnMouse(ev,x,y);
						}

						virtual void OnKey(Key k, wchar_t ch, bool down, bool accel) {
							if(k==KeyCharacter && ch==L' ' && down) {
								// Open the menu when the user hits space
								OnContextMenu(0,0);
							}
							ChildWnd::OnKey(k,ch,down,accel);
						}

						virtual void OnFocus(bool t) {
							Repaint();
							ChildWnd::OnFocus(t);
						}

						virtual void OnContextMenu(Pixels x, Pixels y) {
							ContextMenu cm;

							T val = *(_prop._value);

							typename std::vector<Item>::const_iterator it = _prop._options.begin();
							int i = 0;
							while(it!=_prop._options.end()) {
								const Item& item = *it;
								if(item._separator) {
									cm.AddSeparator(item._name);
								}
								else {
									cm.AddItem(item._name, i, false, item._value==val ? MenuItem::RadioChecked : MenuItem::NotChecked);
								}
								++it;
								++i;
							}

							Area rc = GetClientArea();
							int result = cm.DoContextMenu(this, rc.GetLeft(), rc.GetBottom());
							if(result>=0) {
								const Item& item = _prop._options.at(result);
								_prop.Set(item._value);
							}

							Update();
						}

						virtual void Update() {
							Repaint();
						}

						virtual void OnSize(const Area& ns) {
							Layout();
							Repaint();
						}

						virtual void Layout() {
						}

					protected:
						GenericListProperty<T>& _prop;
						Icon _arrowIcon;
						Icon _icon;
				};

				GenericListProperty(String name, ref<Inspectable> holder, T* value, T def, const String& icon = L""): Property(name), _icon(icon), _holder(holder), _value(value), _default(def) {
				}

				virtual ~GenericListProperty() {
				}

				void AddOption(const String& name, T value) {
					Item it;
					it._name = name;
					it._value = value;
					_options.push_back(it);
				}

				void AddSeparator(const String& title = L"") {
					Item it;
					it._separator = true;
					it._name = title;
					_options.push_back(it);
				}
			
				virtual void Update() {
					if(_pw) {
						_pw->Update();
					}
				}

				virtual ref<Wnd> GetWindow() {
					if(!_pw) {
						_pw = GC::Hold(new PropertyWnd(*this, _icon));
					}

					return _pw;
				}

			protected:
				void Set(const T& value) {
					if(_value!=0 && *_value!=value) {
						UndoBlock::AddAndDoChange(GC::Hold(new PropertyChange<T>(_holder, GetName(), _value, *_value, value)));
					}
				}

				T* _value;
				T _default;
				ref<Inspectable> _holder;
				ref<PropertyWnd> _pw;
				std::vector<Item> _options;
				String _icon;
		};
	}
}

#endif
