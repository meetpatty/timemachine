#include <string.h>

#include <cache.h>
#include <ff.h>
#include <lowio.h>
#include <ms.h>

#include <pspmacro.h>
#include <type_traits>
#include <tm_common.h>

#include "rebootPatches.h"
#include "rebootex_config.h"

#if PSP_MODEL == 0
#include "pspbtcnf_recovery_01g.h"
#define BTCNF_PATH "/kd/pspbtcnf.bin"
#elif PSP_MODEL == 1
#include "pspbtcnf_recovery_02g.h"
#define BTCNF_PATH "/kd/pspbtcnf_02g.bin"
#elif PSP_MODEL == 2
#include "pspbtcnf_recovery_03g.h"
#define BTCNF_PATH "/kd/pspbtcnf_03g.bin"
#endif

namespace {
	using v_iiiiiii_function_t = std::add_pointer_t<void(s32,s32,s32,s32,s32,s32,s32)>;
	inline auto const payloadEntryPtr = reinterpret_cast<v_iiiiiii_function_t const>(0x88600000);

	int recovery;
	int recovery_btcnf_open = 0;

	char path[260];

	void clearCaches() {
		iplKernelDcacheWritebackInvalidateAll();
		iplKernelIcacheInvalidateAll();
	}

	[[noreturn]] inline void payloadEntry(s32 const a0, s32 const a1, s32 const a2, s32 const a3, s32 const t0, s32 const t1, s32 const t2) {
		payloadEntryPtr(a0, a1, a2, a3, t0, t1, t2);

		__builtin_unreachable();
	}

	FATFS fs;
	FIL fp;

	int sceBootLfatMountPatched() {

		if (f_mount(&fs, "ms0:", 1) != FR_OK) {
			return -1;
		}
		
		return 0;
	}

	int sceBootLfatOpenPatched(char *filename) {

		if(strcmp(filename, BTCNF_PATH) == 0) {
			if (recovery == 1) {
				recovery_btcnf_open = 1;
				return 1;
			}
			
			filename[9] = 'd';
		}
		else if(strcmp(filename, "/kd/lfatfs.prx") == 0) {
			strcpy(filename, "/tmctrl500.prx");
		}

		strcpy(path, TM_PATH);
		strcat(path, filename);

		if (f_open(&fp, path, FA_OPEN_EXISTING | FA_READ) == FR_OK) {
			return 1;
		}

		return -1;
	}

	int sceBootLfatReadPatched(void *buffer, int buffer_size) {

		if (recovery_btcnf_open) {
			memcpy(buffer, pspbtcnf_recovery, sizeof(pspbtcnf_recovery));
			return sizeof(pspbtcnf_recovery);
		}

		u32 bytes_read;
		if (f_read(&fp, buffer, buffer_size, &bytes_read) == FR_OK)
			return bytes_read;

		return 0;
	}

	int sceBootLfatClosePatched() {

		if (recovery_btcnf_open)
			recovery_btcnf_open = 0;

		f_close(&fp);
		return 0;
	}
}

int main(s32 const a0, s32 const a1, s32 const a2, s32 const a3, s32 const t0, s32 const t1, s32 const t2) {

	u32 ctrl = _lw(REBOOTEX_PARAM_OFFSET);
	recovery = (ctrl & 0x400) == 0;

	MsLfatFuncs funcs = {
		.msMount = reinterpret_cast<void *>(sceBootLfatMountPatched),
		.msOpen = reinterpret_cast<void *>(sceBootLfatOpenPatched),
		.msRead = reinterpret_cast<void *>(sceBootLfatReadPatched),
		.msClose = reinterpret_cast<void *>(sceBootLfatClosePatched),
	};

	patchIplPayload(&funcs);

	memset(reinterpret_cast<void*>(REBOOTEX_PARAM_OFFSET), 0, 0x100);

	_sw(ctrl & 0xffffff00, REBOOTEX_PARAM_OFFSET);

	payloadEntry(a0, a1, a2, a3, t0, t1, t2);
}