#include "../../include/ui/tjui.h" 
using namespace tj::shared;
using namespace tj::shared::graphics;

#pragma warning(disable: 4355) // 'this' used in initializer list (needed for Animated)

/** TreeNode **/
TreeNode::TreeNode(): _expanded(false), _alwaysExpanded(false), _expandAnimation(this, 0.0) {
}

TreeNode::~TreeNode() {
}

bool TreeNode::IsExpanded() const {
	return _expanded || _alwaysExpanded;
}

bool TreeNode::IsAlwaysExpanded() const {
	return _alwaysExpanded;
}

void TreeNode::SetAlwaysExpanded(bool t) {
	_alwaysExpanded = t;
	if(t) {
		SetExpanded(true, false);
	}
}

void TreeNode::OnAnimationStep(const Animated& which) {
	ref<TreeWnd> parent = _parent;
	if(parent) {
		parent->Repaint();
	}
}

void TreeNode::SetExpanded(bool t, bool recursive) {
	if(!_alwaysExpanded) {
		_expanded = t;

		ref<TreeWnd> parent = _parent;
		if(parent) {
			parent->OnNodeExpanded(*this, t, recursive);
		}

		AnimationBlock ab(Time(300));
		ab.SetEase(Animation::GetEase(Animation::EaseOvershoot));
		_expandAnimation.SetValue(t ? 1.0 : 0.0);
		ab.Commit();
	}
}

void TreeNode::SetParentWindow(ref<TreeWnd> tw) {
	_parent = tw;
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
	child->SetParentWindow(_parent);
	_children.push_back(child);
}

void SimpleTreeNode::SetParentWindow(ref<TreeWnd> tw) {
	TreeNode::SetParentWindow(tw);
	std::deque< strong<TreeNode> >::iterator it = _children.begin();
	while(it!=_children.end()) {
		strong<TreeNode> kid = *it;
		kid->SetParentWindow(tw);
		++it;
	}
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
	g.DrawString(_text.c_str(), (int)_text.length(), theme->GetGUIFont(), SimpleRectangle<float>(row), &sf, &text);
}

void SimpleTreeNode::OnKey(Key k, wchar_t t, bool down, bool isAccelerator) {
}

void SimpleTreeNode::Visit(TreeVisitor& tv) {
	if(tv.Run(*this)) {
		tv.EnterChildren(*this);

		std::deque< strong<TreeNode> >::iterator it = _children.begin();
		while(it!=_children.end()) {
			strong<TreeNode> kid = *it;
			kid->Visit(tv);
			++it;
		}
		tv.LeaveChildren(*this);
	}
}

bool SimpleTreeNode::IsExpandable() const {
	return _children.size() > 0;
}

Pixels SimpleTreeNode::GetHeight(bool r) const {
	if(r) {
		std::deque< strong<TreeNode> >::const_iterator it = _children.begin();
		Pixels h = 0;

		if(IsExpanded()) {
			while(it!=_children.end()) {
				h += (*it)->GetHeight(true);
				++it;
			}
		}

		if(IsAlwaysExpanded()) {
			return h + GetHeight(false);
		}
		else {
			return Pixels(double(h) * _expandAnimation.GetValue()) + GetHeight(false);
		}
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
	if(_root) {
		_root->SetParentWindow(null);
	}

	_root = root;
	_root->SetParentWindow(this);
	UpdateScrollBars();
	Update();
}

void TreeWnd::UpdateScrollBars() {
	Pixels h = 0;
	if(_root) {
		h = _root->GetHeight(true);
		Area ns = GetClientArea();
		SetVerticalScrollInfo(Range<int>(0, h), ns.GetHeight());
	}
}

void TreeWnd::OnSize(const Area& ns) {
	UpdateScrollBars();
	GridWnd::OnSize(ns);
	Repaint();
}

void TreeWnd::OnFocus(bool t) {
	Repaint();
}

void TreeWnd::OnNodeExpanded(TreeNode& node, bool expanded, bool recursive) {
	UpdateScrollBars();
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

			virtual void EnterChildren(TreeNode& tn) {
				if(!tn.IsAlwaysExpanded()) {
					_rc.Narrow(_indent, 0, 0, 0);
				}
			}

			virtual void LeaveChildren(TreeNode& tn) {
				if(!tn.IsAlwaysExpanded()) {
					_rc.Widen(_indent, 0, 0, 0);
				}
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

void TreeWnd::SetSelectedNode(ref<TreeNode> t) {
	_selected = t;
	OnSelectedNodeChanged(t);
	Repaint();
}

void TreeWnd::OnSelectedNodeChanged(ref<TreeNode> tn) {
}

void TreeWnd::SelectNextNode(ref<TreeNode> current) {
	if(!current) return;

	class GetNextVisitor: public TreeVisitor {
		public:
			GetNextVisitor(ref<TreeNode> cursel): _currentlySelected(cursel) {}
			virtual ~GetNextVisitor() {}

			virtual bool Run(TreeNode& node) {
				ref<TreeNode> refNode = &node;
				if(_previous==_currentlySelected) {
					_currentlySelected = refNode;
					return false;
				}
				else {
					_previous = refNode;
					return node.IsExpanded();
				}
			}

			virtual void EnterChildren(TreeNode& tn) {}
			virtual void LeaveChildren(TreeNode& tn) {}

			ref<TreeNode> _currentlySelected;
			ref<TreeNode> _previous;
	};

	GetNextVisitor gtv(current);
	_root->Visit(gtv);
	if(gtv._currentlySelected && gtv._currentlySelected!=current) {
		SetSelectedNode(gtv._currentlySelected);
	}
}

void TreeWnd::SelectPreviousNode(ref<TreeNode> current) {
	if(!current) return;

	class GetPreviousVisitor: public TreeVisitor {
		public:
			GetPreviousVisitor(ref<TreeNode> cursel): _currentlySelected(cursel) {}
			virtual ~GetPreviousVisitor() {}

			virtual bool Run(TreeNode& node) {
				ref<TreeNode> refNode = &node;
				if(refNode==_currentlySelected) {
					_currentlySelected = _previous;
					return false;
				}
				else {
					_previous = refNode;
					return node.IsExpanded();
				}
			}

			virtual void EnterChildren(TreeNode& tn) {}
			virtual void LeaveChildren(TreeNode& tn) {}

			ref<TreeNode> _currentlySelected;
			ref<TreeNode> _previous;
	};

	GetPreviousVisitor gtv(current);
	_root->Visit(gtv);
	if(gtv._currentlySelected && gtv._currentlySelected!=current) {
		SetSelectedNode(gtv._currentlySelected);
	}
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
	else if(k==KeyDown && down && _selected) {
		SelectNextNode(_selected);
	}
	else if(k==KeyUp && down && _selected) {
		SelectPreviousNode(_selected);
	}
	else {
		if(_selected) {
			_selected->OnKey(k,ch,down,accel);
		}
		else {
			GridWnd::OnKey(k,ch,down,accel);
		}
	}
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
				bool isExpandedOrExpanding = node.IsExpanded() || (node._expandAnimation.IsAnimating() && node._expandAnimation > 0.05);
				Pixels h = node.GetHeight(false);
				Pixels hc = node.GetHeight(true);
				Area row = _area;
				_area.Narrow(0, h, 0, 0);
				row.SetY(0);

				if(row.GetHeight() > 0 && row.GetWidth() > 0) {
					row.SetHeight(h);
					
					if(node.IsExpandable()) {
						// Draw lines
						if(isExpandedOrExpanding && !node.IsAlwaysExpanded()) {
							Point start(row.GetLeft() + (_indent/2), row.GetTop() + (h/2));
							Point end(row.GetLeft() + (_indent)/2, row.GetTop() + hc);
							Point secondEnd(end._x + 2, end._y);
							Pen line(_theme->GetColor(Theme::ColorActiveStart), 1.0f);
							_g.DrawLine(&line, start, end);
							_g.DrawLine(&line, end, secondEnd);
						}

						// Draw expand/collapse icon
						if(!node.IsAlwaysExpanded()) {
							Icon& ceIcon = node.IsExpanded() ? _tw._collapseIcon : _tw._expandIcon;
							Area ceArea(row.GetLeft() + ((_indent-16)/2), row.GetTop() + ((h-16)/2), 16, 16);
							ceIcon.Paint(_g, ceArea);
							row.Narrow(_indent, 0, 0, 0);
						}
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
					_g.TranslateTransform(0.0f, (float)h);
					return isExpandedOrExpanding;
				}
				_g.TranslateTransform(0.0f, (float)h);
				return false;
			}

			virtual void EnterChildren(TreeNode& tn) {
				if(!tn.IsAlwaysExpanded()) {
					_area.Narrow(_indent, 0, 0, 0);
				}
				double sc = tn._expandAnimation;
				_g.ScaleTransform((float)1.0, (float)sc);
				_scales.push_back(sc);
			}

			virtual void LeaveChildren(TreeNode& tn) {
				if(!tn.IsAlwaysExpanded()) {
					_area.Widen(_indent, 0, 0, 0);
				}
				double sc = (*_scales.rbegin());
				_g.ScaleTransform((float)1.0, float(1.0/sc));
				_scales.pop_back();
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
			std::deque<double> _scales;
			Area _rc;
			Area _area;
			graphics::Graphics& _g;
			strong<Theme> _theme;
			ColumnInfo& _ci;
			Pixels _indent;
			TreeWnd& _tw;
	};

	if(_root) {
		PaintTreeVisitor painter(g, Area(0,0,area.GetWidth(), area.GetHeight()+GetVerticalPos()), strong<Theme>(theme), *this, *this);
		GraphicsContainer gc = g.BeginContainer();
		g.TranslateTransform(0.0f, (float(-GetVerticalPos()))+3.0f+area.GetTop());
		_root->Visit(painter);
		g.EndContainer(gc);
	}

	GridWnd::Paint(g,theme);
	theme->DrawInsetRectangleLight(g, area);
}

TreeColumnInfo::~TreeColumnInfo() {
}

TreeVisitor::~TreeVisitor() {
}
