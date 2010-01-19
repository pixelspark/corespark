#ifndef _TJPROPERTYSEPARATOR_H
#define _TJPROPERTYSEPARATOR_H

namespace tj {
	namespace shared {
		class EXPORTED PropertySeparator: public Property {
			public:
				PropertySeparator(const String& group, bool collapsed = false);
				virtual ~PropertySeparator();
				virtual ref<Wnd> GetWindow();
				virtual void Update();
				virtual int GetHeight();
				virtual void SetCollapsed(bool c);
				virtual bool IsCollapsed() const;

			protected:
				bool _collapsed;
		};
	}
}

#endif