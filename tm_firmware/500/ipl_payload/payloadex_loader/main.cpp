#include <string.h>
#include <type_traits>

#include <syscon.h>
#include <cache.h>
#include <lowio.h>
#include <pspmacro.h>

#include <tm_common.h>

#if PSP_MODEL == 0
#include <payloadex_01g.h>
#elif PSP_MODEL == 1
#include <payloadex_02g.h>
#endif

namespace {
	using v_uiucuc_function_t = std::add_pointer_t<void(u32,u8,u8)>;
	using v_ipuc_function_t = std::add_pointer_t<void(u32*,u8)>;

#if PSP_MODEL == 0
	inline auto const sceSysconCommonWrite = reinterpret_cast<v_uiucuc_function_t const>(0x04005bc4);
	inline auto const sceSysconCommonRead = reinterpret_cast<v_ipuc_function_t const>(0x04005ac0);
#elif PSP_MODEL == 1
	inline auto const sceSysconCommonWrite = reinterpret_cast<v_uiucuc_function_t const>(0x04006b98);
	inline auto const sceSysconCommonRead = reinterpret_cast<v_ipuc_function_t const>(0x04006a54);
#endif

	void clearCaches() {
      iplKernelDcacheWritebackInvalidateAll();
		iplKernelIcacheInvalidateAll();
	}
}

int main(void) {

	memcpy(reinterpret_cast<void*>(0x08fc0000), payloadex, sizeof(payloadex));

	//Enable ms
   sceSysconCommonWrite(1, 0x4c, 3);
	
	//pspSysconGetCtrl1
	_sw(-1, 0x08fb0000);
	sceSysconCommonRead(reinterpret_cast<u32*>(0x08fb0000), 7);
	
	clearCaches();
	
	return 0;
}
