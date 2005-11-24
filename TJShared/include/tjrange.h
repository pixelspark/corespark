#ifndef _TJRANGE_H
#define _TJRANGE_H

template<typename T> class Range {
	public:
		Range(T start, T end) {
			_start = start;
			_end = end;
		}

		inline T& Start() {
			return _start;
		}

		inline T& End() {
			return _end;
		}
		
		inline bool IsValid() {
			return _start<=_end;
		}

	protected:
		T _start;
		T _end;
};

#endif