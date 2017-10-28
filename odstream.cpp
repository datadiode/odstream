/***********************************************************************/
/*                                                                     */
/* odstream.cpp: Source file for stream class using OutputDebugString  */
/*                                                                     */
/*     Copyright (C) 2011,2017 Yak! / Yasutaka ATARASHI                */
/*                                                                     */
/*     This software is distributed under the terms of a zlib/libpng   */
/*     License.                                                        */
/*                                                                     */
/*     Modified from https://github.com/yak1ex/odstream                */
/*                                                                     */
/***********************************************************************/

#define YAK_DEBUG_NO_HEADER_ONLY
#include "odstream.hpp"

std::ostream& yak::debug_yes::ods() {
	return stream_singleton<debug_yes_impl::odstringbuf>::instance();
}

std::wostream& yak::debug_yes::odw() {
	return stream_singleton<debug_yes_impl::wodstringbuf>::instance();
}

std::ostream& yak::debug_no::pseudo_null_stream<char>::null_stream() {
	return stream_singleton<nullstreambuf>::instance();
}

std::wostream& yak::debug_no::pseudo_null_stream<wchar_t>::null_stream() {
	return stream_singleton<nullstreambuf>::instance();
}
