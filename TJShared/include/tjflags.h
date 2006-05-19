#ifndef _TJFLAGS_H
#define _TJFLAGS_H

template<typename T, typename IntType=int> class Flags {
	public:
		Flags(T flags) {
			_data = flags;
		}

		Flags() {
			_data = (T)0;
		}

		bool IsSet(T flag) const {
			return (IntType(_data) & IntType(flag)) != 0;
		}

		void Set(T flag, bool active) {
			if(active) {
				_data = (T)(IntType(flag)|IntType(_data));
			}
			else {
				_data = (T)(IntType(_data) & (~IntType(flag)));
			}
		}

		const T GetValue() const {
			return _data;
		}

		inline void operator += (T flag) {
			Set(flag, true);
		}

		inline void operator -= (T flag) {
			Set(flag, false);
		}

		inline bool operator[](T flag) {
			return IsSet(flag);
		}

	protected:
		T _data;
};

#endif