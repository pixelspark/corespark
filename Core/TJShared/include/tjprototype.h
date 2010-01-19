#ifndef _TJ_PROTOTYPE_H
#define _TJ_PROTOTYPE_H

#include "tjsharedinternal.h"

namespace tj {
	namespace shared {
			/** Prototypes are a way to be able to create a lot of different types of objects that have a common superclass.
			Suppose you are writing a DMX viewer-like application, in which a multitude of fixtures can be added to a canvas.
			Each fixture has its own class (e.g. for drawing different shapes), but there is one common 'Fixture' superclass.
			The prototype mechanism makes it easier to create fixtures based on some 'prototype name'. This is not all that
			difficult of course, but this little prototype framework adds the ability to dynamically add new 'prototypes' and
			is thus really easy to use in combination with plug-ins. Using this framework, a plug-in only needs a reference to
			the 'fixture factory' (which is a subclass of PrototypeBasedFactory<Fixture>) and it can adds its types, linked to
			its own classes and prototypes.
			**/

			typedef std::wstring PrototypeName;

			template<class P> class Prototype {
				public:
					Prototype(const std::wstring& friendlyName): _friendlyName(friendlyName) {
					}

					virtual ~Prototype() {
					}

					const std::wstring& GetFriendlyName() const {
						return _friendlyName;
					}

					virtual ref<P> CreateInstance() = 0;
					virtual bool Matches(ref<P> object) = 0;

				private:
					std::wstring _friendlyName;
			};

			template<class P, class S> class SubclassedPrototype: public Prototype<S> {
				public:
					SubclassedPrototype(const std::wstring& friendlyName): Prototype<S>(friendlyName) {
					}

					virtual ~SubclassedPrototype() {
					}

					virtual ref<S> CreateInstance() {
						return GC::Hold(new P());
					}

					inline bool Matches(ref<S> object) {
						// IsCastableTo<P> gives some compiler warnings in GCC (not in MSVC though). The dynamic_cast below is equivalent.
						//return object.IsCastableTo<P>();
						return dynamic_cast<P*>(object.GetPointer())!=0;
					}
			};

			template<class T, typename PN = PrototypeName> class PrototypeBasedFactory {
				public:
					inline ref<T> CreateObjectOfType(const PN& wt) {
						typename std::map<PN, ref< Prototype<T> > >::iterator it = _prototypes.find(wt);
						if(it!=_prototypes.end()) {
							ref< Prototype<T> > pr = it->second;
							if(pr) {
								return pr->CreateInstance();
							}
						}
						return null;
					}

					inline PN GetTypeOfObject(ref<T> wt) {
						typename std::map<PN, ref< Prototype<T> > >::iterator it = _prototypes.begin();
						while(it!=_prototypes.end()) {
							ref<Prototype<T> > pr = it->second;
							if(pr && pr->Matches(wt)) {
								return it->first;
							}
							++it;
						}
						return PN();
					}
				
					inline bool KnowsType(const PN& prototypeName) {
							return _prototypes.find(prototypeName) != _prototypes.end();
					}

					inline void RegisterPrototype(const PN& prototypeName, ref< Prototype<T> > pt) {
						if(pt) {
							_prototypes[prototypeName] = pt;
						}
					}

				protected:
					std::map<PN, ref< Prototype<T> > > _prototypes;
			};
	}
}

#endif