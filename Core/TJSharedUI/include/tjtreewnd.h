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
 
 #ifndef _TJTREEWND_H
#define _TJTREEWND_H

namespace tj {
	namespace shared {
		class TreeNode;
		class TreeWnd;

		class EXPORTED TreeColumnInfo {
			public:
				virtual ~TreeColumnInfo();
				virtual Area GetFieldArea(const Area& row, int columnID) const = 0;
				virtual bool IsColumnVisible(int columnID) const = 0;
		};
		
		class EXPORTED TreeVisitor {
			public:
				virtual ~TreeVisitor();
				virtual bool Run(TreeNode& t) = 0;
				virtual void EnterChildren(TreeNode& t) = 0;
				virtual void LeaveChildren(TreeNode& t) = 0;
		};

		class EXPORTED TreeNode: public virtual Object, public Animatable {
			friend class TreeWnd;
			friend class SimpleTreeNode;

			public:
				TreeNode();
				virtual ~TreeNode();
				virtual bool IsExpanded() const;
				virtual void SetExpanded(bool t, bool recursive = false);
				virtual bool IsAlwaysExpanded() const;
				virtual void SetAlwaysExpanded(bool t);

				virtual bool IsExpandable() const = 0;
				virtual Pixels GetHeight(bool recursive) const = 0;
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme, const Area& row, const TreeColumnInfo& ci) = 0;
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y) = 0;
				virtual void OnKey(Key k, wchar_t t, bool down, bool isAccelerator) = 0;
				virtual void Visit(TreeVisitor& tv) = 0;
				
				virtual void OnAnimationStep(const Animated& which);

			protected:
				virtual void SetParentWindow(ref<TreeWnd> parent);
				weak<TreeWnd> _parent;

			private:
				bool _expanded;
				bool _alwaysExpanded;
				Animated _expandAnimation;
		};

		class EXPORTED SimpleTreeNode: public TreeNode {
			public:
				SimpleTreeNode(const String& text = L"");
				virtual ~SimpleTreeNode();
				virtual bool IsExpandable() const;
				virtual Pixels GetHeight(bool recursive) const;
				virtual void SetHeight(Pixels c);
				virtual void SetText(const String& text);
				virtual void Add(strong<TreeNode> child);
				virtual void Paint(graphics::Graphics& g, ref<Theme> theme, const Area& row, const TreeColumnInfo& ci);
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnKey(Key k, wchar_t t, bool down, bool isAccelerator);
				virtual void SetExpanded(bool t, bool recursive);
				virtual void Visit(TreeVisitor& tv);

			protected:
				virtual void SetParentWindow(ref<TreeWnd> parent);

				std::deque< strong<TreeNode> > _children;
				Pixels _height;
				String _text;
		};

		class EXPORTED TreeWnd: public GridWnd {
			friend class TreeNode;

			public:
				TreeWnd();
				virtual ~TreeWnd();
				virtual void Paint(graphics::Graphics& g, strong<Theme> theme);
				virtual void SetRoot(ref<TreeNode> _root);
				virtual ref<TreeNode> GetRoot();
				virtual void OnMouse(MouseEvent ev, Pixels x, Pixels y);
				virtual void OnSize(const Area& ns);
				virtual void OnScroll(ScrollDirection dir);
				virtual void OnFocus(bool t);
				virtual void OnKey(Key k, wchar_t ch, bool down, bool accel);
				virtual void SetSelectedNode(ref<TreeNode> tn);

			protected:
				enum HitType {
					HitTypeNone = 0,
					HitTypeExpand,
					HitTypeCollapse,
					HitTypeItem,
				};
				
				virtual void OnNodeExpanded(TreeNode& node, bool expanded, bool recursive);
				virtual void OnSelectedNodeChanged(ref<TreeNode> current);
				virtual void UpdateScrollBars();
				virtual ref<TreeNode> GetTreeNodeAt(Pixels x, Pixels y, Area& row, HitType& ht);
				virtual void SelectNextNode(ref<TreeNode> current);
				virtual void SelectPreviousNode(ref<TreeNode> current);
				ref<TreeNode> _root;
				ref<TreeNode> _selected;
				Icon _expandIcon, _collapseIcon;
		};
	}
}

#endif