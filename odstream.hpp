/***********************************************************************/
/*                                                                     */
/* odstream.hpp: Header file for stream class using OutputDebugString  */
/*                                                                     */
/*     Copyright (C) 2011,2017 Yak! / Yasutaka ATARASHI                */
/*                                                                     */
/*     This software is distributed under the terms of a zlib/libpng   */
/*     License.                                                        */
/*                                                                     */
/*     Modified from https://github.com/yak1ex/odstream                */
/*                                                                     */
/***********************************************************************/

#ifndef ODSTREAM_HPP
#define ODSTREAM_HPP

#include <sstream>
#include <windows.h>

#define ODS(vv) YAK_DEBUG_EVAL (yak::auto_lock(), yak::debug::ods() vv).flush()
#define ODS_NOFLUSH(vv) YAK_DEBUG_EVAL (yak::auto_lock(), yak::debug::ods() vv)
#define ODW(vv) YAK_DEBUG_EVAL (yak::auto_lock(), yak::debug::odw() vv).flush()
#define ODW_NOFLUSH(vv) YAK_DEBUG_EVAL (yak::auto_lock(), yak::debug::odw() vv)

namespace yak {

	// Using this facility in a thread-safe manner requires external locking,
	// which the above defined macros apply with the help of class auto_lock.
	class auto_lock
	{
		CRITICAL_SECTION *const p;
	public:
		auto_lock(CRITICAL_SECTION *q = global()) : p(q) {
			EnterCriticalSection(p);
		}
		~auto_lock() {
			LeaveCriticalSection(p);
		}
	private:
		// Hide copy constructor and assignment operator
		auto_lock(const auto_lock&);
		auto_lock& operator=(const auto_lock&);
		// A global CRITICAL_SECTION which the auto_lock refers to by default
		static CRITICAL_SECTION *global() {
			static CRITICAL_SECTION *volatile pcs = 0;
			while (pcs == 0) {
				static LONG stage = 0;
				if (InterlockedCompareExchange(&stage, 1, 0) == 0) {
					static CRITICAL_SECTION cs;
					InitializeCriticalSection(&cs);
					pcs = &cs;
				} else {
					Sleep(0);
				}
			}
			return pcs;
		}
	};

	// Use the Construct On First Use Idiom, to avoid static initialization order fiasco
	template<typename T>
	class stream_singleton
	{
		T buf;
		typename T::stream_type stm;
		stream_singleton() : stm(&buf) { }
		// Provide a static heap so as to not trigger memory leak detection
		void *operator new(size_t n) {
			static union {
				double alignment;
				char placeholder[sizeof(stream_singleton)];
			} heap;
			return n == sizeof heap.placeholder ? heap.placeholder : 0;
		}
	public:
		static typename T::stream_type &instance() {
			// Allocate instance on heap to prevent destruction during CRT cleanup
			static stream_singleton *instance = 0;
			if (instance == 0)
				instance = new stream_singleton;
			return instance->stm;
		}
	};

	namespace debug_yes {

		class debug_yes_impl
		{
			class odstringbuf : public std::stringbuf
			{
			public:
				typedef std::ostream stream_type;
				virtual int sync() {
					OutputDebugStringA(str().c_str());
					str("");
					return 0;
				}
			};
			class wodstringbuf : public std::wstringbuf
			{
			public:
				typedef std::wostream stream_type;
				virtual int sync() {
					OutputDebugStringW(str().c_str());
					str(L"");
					return 0;
				}
			};
			friend std::ostream& ods();
			friend std::wostream& odw();
		}; // struct debug_yes_impl
#ifdef YAK_DEBUG_NO_HEADER_ONLY
		std::ostream& ods();
		std::wostream& odw();
#else // ifdef YAK_DEBUG_NO_HEADER_ONLY
		inline std::ostream& ods() {
			return stream_singleton<debug_yes_impl::odstringbuf>::instance();
		}
		inline std::wostream& odw() {
			return stream_singleton<debug_yes_impl::wodstringbuf>::instance();
		}
#endif // ifdef YAK_DEBUG_NO_HEADER_ONLY
	} // namespace debug_yes

	namespace debug_no {

		template<typename TChar>
		class pseudo_null_stream
		{
		private:
			class nullstreambuf : public std::basic_streambuf<TChar, std::char_traits<TChar> >
			{
			public:
				typedef std::basic_ostream<TChar, std::char_traits<TChar> > stream_type;
			};
#ifdef YAK_DEBUG_NO_HEADER_ONLY
			static std::basic_ostream<TChar, std::char_traits<TChar> >& null_stream();
#else // ifdef YAK_DEBUG_NO_HEADER_ONLY
			static std::basic_ostream<TChar, std::char_traits<TChar> >& null_stream() {
				stream_singleton<nullstreambuf>::instance();
			}
#endif // ifdef YAK_DEBUG_NO_HEADER_ONLY
		public:
			template<typename T>
			const pseudo_null_stream& operator << (T) const { return *this; }
			const pseudo_null_stream& operator << (std::ostream& (*)(std::ostream&)) const { return *this; }
			const pseudo_null_stream& operator << (std::ios_base& (*)(std::ios_base&)) const { return *this; }
			const pseudo_null_stream& operator << (std::basic_ios<char>& (*)(std::basic_ios<char>&)) const { return *this; }
			const pseudo_null_stream& flush() const { return *this; }
			operator std::basic_ostream<TChar, std::char_traits<TChar> >& () const { return null_stream(); }
		};

		typedef pseudo_null_stream<char> ods;
		typedef pseudo_null_stream<wchar_t> odw;

	} // namespace debug_no

#ifndef YAK_DEBUG_NO_DECLARE_NAMESPACE
#ifdef YAK_DEBUG_EVAL
	namespace debug = debug_yes;
#else
	namespace debug = debug_no;
#endif
#endif

} // namespace yak

#ifdef YAK_DEBUG_EVAL
#	define YAK_DEBUG_EVAL // Let evaluation of subsequent expression take place
#else
#	define YAK_DEBUG_EVAL sizeof // Prevent evaluation of subsequent expression
#endif

#endif
