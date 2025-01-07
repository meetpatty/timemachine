/* 
 * Based on rebootex/main.c from minimum edition - https://github.com/PSP-Archive/minimum_edition
*/

#include <string.h>
#include <type_traits>

#include <cache.h>
#include <ff.h>
#include <lowio.h>
#include <ms.h>
#include <syscon.h>

#include <pspmacro.h>
#include <tm_common.h>

#include "rebootPatches.h"
#include "rebootex_config.h"
#include "btcnf.h"

#if PSP_MODEL == 0
#define BTCNF_PATH "/kd/pspbtcnf.bin"
#elif PSP_MODEL == 1
#define BTCNF_PATH "/kd/pspbtcnf_02g.bin"
#endif

namespace {

	using v_iiiiiii_function_t = std::add_pointer_t<void(s32,s32,s32,s32,s32,s32,s32)>;
	inline auto const rebootEntryPtr = reinterpret_cast<v_iiiiiii_function_t const>(0x88600000);

	int btcnf_load_flag;
	int rtm_flag;

	char *systemctrl = (char *)0x88FB0100;
	u32 sizeSystemctrl = 0;

	char *onRebootAfter = NULL;
	void *onRebootBuf = NULL;
	u32 onRebootSize = 0;
	u32 onRebootFlag = 0;

	int bootIndex = 0;

	char path[260];

	[[noreturn]] inline void rebootEntry(s32 const a0, s32 const a1, s32 const a2, s32 const a3, s32 const t0, s32 const t1, s32 const t2) {
		rebootEntryPtr(a0, a1, a2, a3, t0, t1, t2);

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

		if (strcmp(filename, BTCNF_PATH) == 0)
		{
			if (bootIndex == 0)
				filename[9] = 'j';
			else if (bootIndex == 1)
				filename[9] = 'k';
			else if (bootIndex == 2)
				filename[9] = 'l';
			
			if (onRebootAfter)
				btcnf_load_flag = 1;
		}
		else if (strcmp(filename, "/rtm.prx") == 0)
		{
			rtm_flag = 1;
			return 0;
		}
		else if (strcmp(filename, "/kd/lfatfs.prx") == 0) {
			strcpy(filename, "/tmctrl500.prx");
		}

		strcpy(path, TM_PATH);
		strcat(path, filename);

		if (f_open(&fp, path, FA_OPEN_EXISTING | FA_READ) == FR_OK) {
			return 0;
		}

		return -1;
	}

	int sceBootLfatReadPatched(void *buffer, int buffer_size) {

		if (rtm_flag)
		{
			int load_size = onRebootSize;

			if( load_size > buffer_size)
				load_size = buffer_size;

			memcpy(buffer, onRebootBuf, load_size);

			onRebootSize -= load_size;
			onRebootBuf += load_size;
			return load_size;
		}

		u32 ret;
		f_read(&fp, buffer, buffer_size, &ret);

		if (btcnf_load_flag)
		{
			btcnf_load_flag = 0;

			module_rtm[0].before_path = onRebootAfter;
			module_rtm[0].flag = onRebootFlag;

			ret = btcnf_patch(buffer, ret, module_rtm, 0, 0);
		}

		return ret;
	}

	int sceBootLfatClosePatched() {

		if(rtm_flag)
		{
			rtm_flag = 0;
			return 0;
		}

		f_close(&fp);
	
		return 0;
	}
}

int main(s32 const a0, s32 const a1, s32 const a2, s32 const a3, s32 const t0, s32 const t1, s32 const t2) {

	struct RebootexParam *rebootex_param = reinterpret_cast<RebootexParam *>(REBOOTEX_PARAM_OFFSET);

	bootIndex = rebootex_param->reboot_index;

	onRebootAfter = reinterpret_cast<char *>(rebootex_param->on_reboot_after);
	onRebootBuf	= rebootex_param->on_reboot_buf;
	onRebootSize	= rebootex_param->on_reboot_size;
	onRebootFlag	= rebootex_param->on_reboot_flag;

	btcnf_load_flag = 0;
	rtm_flag = 0;

	MsLfatFuncs funcs = {
		.msMount = reinterpret_cast<void *>(sceBootLfatMountPatched),
		.msOpen = reinterpret_cast<void *>(sceBootLfatOpenPatched),
		.msRead = reinterpret_cast<void *>(sceBootLfatReadPatched),
		.msClose = reinterpret_cast<void *>(sceBootLfatClosePatched),
	};

	patchRebootBin(&funcs);

	// Hmm?
	iplSysregSpiClkEnable(ClkSpi::SPI1);

	iplSysregGpioIoEnable(GpioPort::SYSCON_REQUEST);
	iplSysregGpioIoEnable(GpioPort::MS_LED);
	iplSysregGpioIoEnable(GpioPort::WLAN_LED);
	sdkSync();

	iplSysconInit();
	iplSysconCtrlMsPower(true);
	
	rebootEntry(a0, a1, a2, a3, t0, t1, t2);
}