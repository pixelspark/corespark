#ifndef _TJGC_H
#define _TJGC_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED GC {
	public:
		static void IncrementLive(size_t size=0);
		static void DecrementLive(size_t size=0);
		static long GetLiveCount();
		static long GetSize();
		template<typename T> static ref< T > Hold(T* x);

		static void AddLog(void* id, std::wstring info);
		static void RemoveLog(void* id);

	protected:
		static std::map< void*, std::wstring> _objects;
};

template<typename T> ref<T> GC::Hold(T* x) {
	// get type information
	std::string name = typeid(x).name();

	wchar_t* buf  = new wchar_t[name.length()+2];
	mbstowcs_s(0, buf, name.length()+1, name.c_str(), _TRUNCATE);
	AddLog((void*)x, std::wstring(buf));
	delete[] buf;

	Resource<T>* rs = new Resource<T>(x);
	return rs->Reference();
}

#pragma warning(pop)
#endif