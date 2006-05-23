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

		inline void SetStart(T start) {
			_start = start;
		}

		inline void SetEnd(T end) {
			_end = end;
		}
		
		inline bool IsValid() {
			return _start<=_end;
		}

		inline T Length() {
			return _end - _start;
		}

		template<typename T> static Range<T>& Widest(Range<T> a, Range<T> b) {
			return a.Length()>b.Length()?a:b;
		}

		template<typename T> static Range<T>& Narrowest(Range<T> a, Range<T> b) {
			return a.Length()<b.Length()?a:b;
		}

	protected:
		T _start;
		T _end;
};

#endif