#include "../../include/ui/tjui.h" 
using namespace tj::shared;
using namespace tj::shared::graphics;

TreeColumnInfo::~TreeColumnInfo() {
}

TreeVisitor::~TreeVisitor() {
}

/** TreeNode **/
TreeNode::TreeNode(): _expanded(false), _expandAnimation(this, 0.0) {
}

TreeNode::~TreeNode() {
}

bool TreeNode::IsExpanded() const {
	return _expanded;
}

void TreeNode::OnAnimationStep(const Animated& which) {
	Log::Write(L"TJShared/TreeNode", L"OnAnimationStep; current value = "+Stringify(_expandAnimation.GetValue()));
}

void TreeNode::SetExpanded(bool t, bool recursive) {
	_expanded = t;

	AnimationBlock ab;
	_expandAnimation.SetValue(t ? 1.0 : 0.0);
	ab.Commit();
}

/** SimpleTreeNode **/
SimpleTreeNode::SimpleTreeNode(const std::wstring& text): _height(-1), _text(text) {
}

SimpleTreeNode::~SimpleTreeNode() {
}

void SimpleTreeNode::SetHeight(Pixels c) {
	_height = c;
}

void SimpleTreeNode::Add(strong<TreeNode> child) {
	if(ref<TreeNode>(child).GetPointer()==dynamic_cast<TreeNode*>(this)) {
		Throw(L"Cannot add tree node to itself as a child", ExceptionTypeSevere);
	}
	_children.push_back(child);
}

void SimpleTreeNode::SetText(const std::wstring& text) {
	_text = text;
}

void SimpleTreeNode::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
}

void SimpleTreeNode::SetExpanded(bool t, bool recursive) {
	if(recursive) {
		std::deque< strong<TreeNode> >::iterator it = _children.begin();
		while(it!=_children.end()) {
			strong<TreeNode> kid = *it;
			kid->SetExpanded(t, recursive);
			++it;
		}
	}
	
	TreeNode::SetExpanded(t);
}

void SimpleTreeNode::Paint(graphics::Graphics& g, ref<Theme> theme, const Area& row, const TreeColumnInfo& ci) {
	// Draw self
	SolidBrush text(theme->GetColor(Theme::ColorText));
	StringFormat sf;
	sf.SetFormatFlags(sf.GetFormatFlags()|StringFormatFlagsLineLimit);
	sf.SetLineAlignment(StringAlignmentCenter);
	g.DrawString(_text.c_str(), (int)_text.length(), theme->GetGUIFont(), BasicRectangle<float>(row), &sf, &text);
}

void SimpleTreeNode::Visit(TreeVisitor& tv) {
	if(tv.Run(*this)) {
		tv.EnterChildren();

		std::deque< strong<TreeNode> >::iterator it = _children.begin();
		while(it!=_children.end()) {
			strong<TreeNode> kid = *it;
			kid->Visit(tv);
			++it;
		}
		tv.LeaveChildren();
	}
}

bool SimpleTreeNode::IsExpandable() const {
	return _children.size() > 0;
}

Pixels SimpleTreeNode::GetHeight(bool r) const {
	if(r) {
		std::deque< strong<TreeNode> >::const_iterator it = _children.begin();
		Pixels h = GetHeight(false);

		if(IsExpanded()) {
			while(it!=_children.end()) {
				h += (*it)->GetHeight(true);
				++it;
			}
		}

		return h;
	}
	else {
		if(_height>0) {
			return _height;
		}
		else {
			return ThemeManager::GetTheme()->GetMeasureInPixels(Theme::MeasureListItemHeight);
		}
	}
}

TreeWnd::TreeWnd(): _expandIcon(Icons::GetIconPath(Icons::IconExpand)), _collapseIcon(Icons::GetIconPath(Icons::IconCollapse)) {
	SetVerticallyScrollable(true);
}

TreeWnd::~TreeWnd() {
}

void TreeWnd::SetRoot(ref<TreeNode> root) {
	_root = root;
	UpdateScrollBars();
	Update();
}

void TreeWnd::UpdateScrollBars() {
	Pixels h = 0;
	if(_root) {
		h = _root->GetHeight(true);
	}
	Area ns = GetClientArea();
	SetVerticalScrollInfo(Range<int>(0, h), ns.GetHeight());
}

void TreeWnd::OnSize(const Area& ns) {
	UpdateScrollBars();
	GridWnd::OnSize(ns);
	Repaint();
}

void TreeWnd::OnFocus(bool t) {
	Repaint();
}

void TreeWnd::OnScroll(ScrollDirection dir) {
	GridWnd::OnScroll(dir);
	Repaint();
}

ref<TreeNode> TreeWnd::GetRoot() {
	return _root;
}

ref<TreeNode> TreeWnd::GetTreeNodeAt(Pixels x, Pixels y, Area& row, HitType& ht) {
	if(!_root) {
		return null;
	}

	y += GetVerticalPos();

	strong<Theme> theme = ThemeManager::GetTheme();
	Pixels indent = theme->GetMeasureInPixels(Theme::MeasureListItemHeight);

	class HitTestVisitor: public TreeVisitor {
		public:
			HitTestVisitor(Pixels x, Pixels y, const Area& rc, Pixels indent): _indent(indent), _x(x), _y(y), _rc(rc), _ht(HitTypeNone) {
			}

			virtual ~HitTestVisitor() {
			}

			virtual bool Run(TreeNode& node) {
				Pixels h = node.GetHeight(false);
				Area row = _rc;
				row.SetHeight(h);
				_rc.Narrow(0, h, 0, 0);
				
				if(row.IsInside(_x, _y)) {
					// found, it's this item
					if(node.IsExpandable() && (_x < (row.GetLeft() + _indent)) /* && (_y < (row.GetTop()+_indent)) */) {
						// Expand/collapse
						_ht = node.IsExpanded() ? HitTypeCollapse : HitTypeExpand;
					}
					else {
						_ht = HitTypeItem;
					}

					_hit = &node;
					return false;
				}
				else {
					return node.IsExpanded();
				}
			}

			virtual void EnterChildren() {
				_rc.Narrow(_indent, 0, 0, 0);
			}

			virtual void LeaveChildren() {
				_rc.Widen(_indent, 0, 0, 0);
			}

			Area _rc;
			Pixels _x, _y;
			ref<TreeNode> _hit;
			Pixels _indent;
			HitType _ht;
	};

	HitTestVisitor htv(x, y, GetClientArea(), indent);
	_root->Visit(htv);
	row = htv._rc;
	ht = htv._ht;
	return htv._hit;
}

void TreeWnd::OnKey(Key k, wchar_t ch, bool down, bool accel) {
	if(k==KeyRight && down) {
		if(_selected && _selected->IsExpandable()) {
			_selected->SetExpanded(true, IsKeyDown(KeyControl));
		}
	}
	else if(k==KeyLeft && down) {
		if(_selected && _selected->IsExpandable()) {
			_selected->SetExpanded(false, IsKeyDown(KeyControl));
		}
	}
	GridWnd::OnKey(k,ch,down,accel);
}

void TreeWnd::OnMouse(MouseEvent ev, Pixels x, Pixels y) {
	if(ev==MouseEventLDown) {
		Focus();
	}

	Area rc = GetClientArea();
	if(y > rc.GetTop()) {
		Area row;
		HitType ht = HitTypeNone;
		ref<TreeNode> tn = GetTreeNodeAt(x,y, row, ht);
		if(tn) {
			if(ev==MouseEventLDown) {
				if(ht==HitTypeCollapse) {
					tn->SetExpanded(false, IsKeyDown(KeyControl));
					UpdateScrollBars();
				}
				else if(ht==HitTypeExpand) {
					tn->SetExpanded(true, IsKeyDown(KeyControl));
					UpdateScrollBars();
				}
				else if(ht==HitTypeItem) {
					_selected = tn;
				}
			}
			tn->OnMouse(ev, x - row.GetLeft(), y - row.GetTop());

			if(ev!=MouseEventMove) {
				Repaint();
			}
		}
	}
	GridWnd::OnMouse(ev,x,y);
}

void TreeWnd::Paint(graphics::Graphics& g, strong<Theme> theme) {
	Area area = GetClientArea();
	SolidBrush back(theme->GetColor(Theme::ColorBackground));
	SolidBrush disabled(theme->GetColor(Theme::ColorDisabledOverlay));
	g.FillRectangle(&back, area);

	class PaintTreeVisitor: public TreeVisitor, public TreeColumnInfo {
		public:
			PaintTreeVisitor(graphics::Graphics& g, const Area& area, strong<Theme> theme, ColumnInfo& ci, TreeWnd& tw): _tw(tw), _area(area), _rc(area), _ci(ci), _g(g), _theme(theme) {
				_indent = _theme->GetMeasureInPixels(Theme::MeasureListItemHeight);
			}

			virtual ~PaintTreeVisitor() {
			}

			virtual bool Run(TreeNode& node) {
				Pixels h = node.GetHeight(false);
				Pixels hc = node.GetHeight(true);
				Area row = _area;
				_area.Narrow(0, h, 0, 0);

				if(row.GetHeight() > 0 && row.GetWidth() > 0) {
					row.SetHeight(h);
					
					if(node.IsExpandable()) {
						// Draw lines
						if(node.IsExpanded()) {
							Point start(row.GetLeft() + (_indent/2), row.GetTop() + (h/2));
							Point end(row.GetLeft() + (_indent)/2, row.GetTop() + hc);
							Point secondEnd(end._x + 2, end._y);
							Pen line(_theme->GetColor(Theme::ColorActiveStart), 1.0f);
							_g.DrawLine(&line, start, end);
							_g.DrawLine(&line, end, secondEnd);
						}

						// Draw expand/collapse icon
						Icon& ceIcon = node.IsExpanded() ? _tw._collapseIcon : _tw._expandIcon;
						Area ceArea(row.GetLeft() + ((_indent-16)/2), row.GetTop() + ((h-16)/2), 16, 16);
						ceIcon.Paint(_g, ceArea);
						row.Narrow(_indent, 0, 0, 0);
					}

					if(_tw._selected.GetPointer()==&node) {
						LinearGradientBrush colorSelected(PointF(0.0f, float(row.GetTop()-1)), PointF(0.0f, float(row.GetBottom()+1)), _theme->GetColor(Theme::ColorTimeSelectionStart), _theme->GetColor(Theme::ColorTimeSelectionEnd));
						_g.FillRectangle(&colorSelected, row);
						
						if(!_tw.HasFocus()) {
							SolidBrush disabled(_theme->GetColor(Theme::ColorDisabledOverlay));
							_g.FillRectangle(&disabled, row);
						}
					}
					
					node.Paint(_g, _theme, row, *this);
					return node.IsExpanded();
				}
				return false;
			}

			virtual void EnterChildren() {
				_area.Narrow(_indent, 0, 0, 0);
			}

			virtual void LeaveChildren() {
				_area.Widen(_indent, 0, 0, 0);
			}

			virtual Area GetFieldArea(const Area& row, int columnID) const {
				Pixels w = _rc.GetWidth();
				Pixels colLeft = Pixels(w*_ci.GetColumnX(columnID)) + _rc.GetLeft();
				Pixels colW = Pixels(w*_ci.GetColumnWidth(columnID));

				// Take the intersection rectangle (on the x-axis) of the 'real' column and the row
				Pixels leftBound = max(colLeft, row.GetLeft());
				Pixels rightBound = min(colLeft+colW, row.GetRight());

				Area fa = Area(leftBound, row.GetTop(), rightBound-leftBound, row.GetHeight());
				if(fa.GetWidth()>0 && fa.GetHeight()>0) {
					return fa;
				}
				return Area(0,0,0,0);
			}

			virtual bool IsColumnVisible(int columnID) const {
				return _ci.IsColumnVisible(columnID);
			}

		protected:
			Area _rc;
			Area _area;
			graphics::Graphics& _g;
			strong<Theme> _theme;
			ColumnInfo& _ci;
			Pixels _indent;
			TreeWnd& _tw;
	};

	if(_root) {
		Area scrolled = area;
		scrolled.Widen(0,GetVerticalPos(),0,0);
		PaintTreeVisitor painter(g, scrolled, strong<Theme>(theme), *this, *this);
		_root->Visit(painter);
	}

	GridWnd::Paint(g,theme);
	theme->DrawInsetRectangleLight(g, area);
}