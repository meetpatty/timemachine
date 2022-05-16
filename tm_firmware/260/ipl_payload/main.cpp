#include <string.h>

#include <pspmacro.h>
#include <type_traits>

#include <tm_common.h>
#include <payloadex.h>

namespace {
	using v_v_function_t = std::add_pointer_t<void()>;

	inline auto const cache1Ptr = reinterpret_cast<v_v_function_t const>(0x040011c0);
	inline auto const cache2Ptr = reinterpret_cast<v_v_function_t const>(0x04001150);
	inline auto const mainEntryPtr = reinterpret_cast<v_v_function_t const>(0x04000000);

	[[noreturn]] inline void mainEntry() {

		asm("li	$sp, 0x040fff00");
		mainEntryPtr();

		__builtin_unreachable();
	}

	void clearCaches() {
		cache1Ptr();
		cache2Ptr();
	}

	void loadPayloadEx() {

		//Copy payloadex to 0x883e0000
		memcpy((void*)0x083e0000, payloadex, sizeof(payloadex));
		clearCaches();		
	}
}

int main() {

	MAKE_CALL(0x0400009c, loadPayloadEx);

	//Change payload entry point to payloadex
	_sw(0x3C19883E, 0x04008588);

	clearCaches();

	mainEntry();
}