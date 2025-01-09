#include <string.h>
#include <type_traits>

#include <syscon.h>
#include <cache.h>
#include <lowio.h>
#include <pspmacro.h>

#include <tm_common.h>

#if PSP_MODEL == 0
#include <payloadex_loader_01g.h>
#define LD_PAYLOADEX_ADDR   0x0400c534
#define CLR_SCRATCHPAD_ADDR 0x04001100
#elif PSP_MODEL == 1
#include <payloadex_loader_02g.h>
#define LD_PAYLOADEX_ADDR   0x0400d534
#define CLR_SCRATCHPAD_ADDR 0x040011e4
#define SET_KEYS_ADDRESS    0x04001144
#elif PSP_MODEL == 2
#include <payloadex_loader_03g.h>
#define LD_PAYLOADEX_ADDR   0x0400d534
#define CLR_SCRATCHPAD_ADDR 0x04001218
#define SET_KEYS_ADDRESS    0x04001178
#endif

namespace {
	using v_v_function_t = std::add_pointer_t<void()>;

	inline auto const mainEntryPtr = reinterpret_cast<v_v_function_t const>(0x04000000);

	[[noreturn]] inline void mainEntry() {

		asm("li	$sp, 0x040fff00");
		mainEntryPtr();

		__builtin_unreachable();
	}

	void clearCaches() {
		iplKernelDcacheWritebackInvalidateAll();
		iplKernelIcacheInvalidateAll();
	}

#ifdef SET_KEYS_ADDRESS
	u32 key[] =
	{
#if PSP_MODEL == 1
		0x802a43ad, 0xa570d79a, 0x890e5087, 0xc6055fe5,
		0xb7558656, 0x89d4608b, 0x83d431ea, 0x469dc537,
#elif PSP_MODEL == 2
		0x6e85db79, 0x3af7377a, 0xc404855d, 0xa3bc96c3,
		0x112603fb, 0x5afd5d28, 0xe3a192e9, 0xf39ef7f7,
#endif
		0x0, 0x0, 0x0, 0x0,
		0x0, 0x0, 0x0, 0x0
	};

	int setkey() {

		for (int i = 0; i < sizeof(key)/sizeof(*key); i++)
			reinterpret_cast<u32*>(0xbfc00200)[i] = key[i];

		return 0;
	}

#endif

	void *my_memcpy(void *m1, const void *m2, size_t size)
	{
		int i;
		u8 *p1 = (u8 *)m1;
		u8 *p2 = (u8 *)m2;

		for (i = 0; i < size; i++)
		{
			p1[i] = p2[i];
		}

		return m1;
	}

}

int main() {

	my_memcpy(reinterpret_cast<void*>(0x010000), payloadex, sizeof(payloadex));

	MAKE_CALL(0x0400035c, 0x10000);

	//Change payload entry point to payloadex
	_sw(LUI(GPREG_T9, 0x88fc), LD_PAYLOADEX_ADDR);

	_sw(0, CLR_SCRATCHPAD_ADDR);

#ifdef SET_KEYS_ADDRESS
	MAKE_JUMP(SET_KEYS_ADDRESS, setkey);
#endif

	clearCaches();

	mainEntry();
}