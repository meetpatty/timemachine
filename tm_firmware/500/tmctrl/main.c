#include <pspkernel.h>
#include <pspsysevent.h>
#include <pspthreadman_kernel.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <systemctrl.h>
#include <pspmacro.h>

#include <stdio.h>
#include <string.h>

#include "main.h"
#include <flashemu.h>
#include <rebootex.h>

PSP_MODULE_INFO("TimeMachine_Control", 0x1007, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

STMOD_HANDLER previous;

void ClearCaches()
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheInvalidateAll();
}

int sceKernelGzipDecompressPatched(u8 *dest, int destSize, u8 *src, u32 unknown)
{
	switch(sceKernelGetModel()) {
		case 0:
			src = rebootex_01g;
			break;
		case 1:
			src = rebootex_02g;
			break;
	}

	return sceKernelGzipDecompress(dest, destSize, src, 0);
}

void PatchSystemControl()
{
	SceModule2 *mod = (SceModule2 *)sceKernelFindModuleByName("SystemControl");

	if(sceKernelGetModel() == 0)
	{
		MAKE_CALL(mod->text_addr + 0x2408, sceKernelGzipDecompressPatched);
	}
	else
	{
		MAKE_CALL(mod->text_addr + 0x2690, sceKernelGzipDecompressPatched);
	}
	
	ClearCaches();
}

int OnModuleStart(SceModule2 *mod)
{
	char *modname = mod->modname;

	if(strcmp(modname, "sceMediaSync") == 0)
	{
		SceModule2 *msfat_drv = (SceModule2 *)sceKernelFindModuleByName("sceMSFAT_Driver");

		MAKE_CALL(msfat_drv->text_addr + 0x48D4, df_openPatched);
		MAKE_CALL(msfat_drv->text_addr + 0x5338, df_dopenPatched);
		MAKE_CALL(msfat_drv->text_addr + 0x5B90, df_devctlPatched);

		ClearCaches();
	}
	else if(strcmp(modname, "sceLflashFatfmt") == 0)
	{
		u32 sceLflashFatfmtStartFatfmt = FindProc("sceLflashFatfmt", "LflashFatfmt", 0xB7A424A4);
		if(sceLflashFatfmtStartFatfmt)
		{
			MAKE_FUNCTION_RETURN0(sceLflashFatfmtStartFatfmt);
			ClearCaches();
		}
	}

	if(!previous)
		return 0;

	return previous(mod);
}

//0x000000E0
int module_start(SceSize args, void *argp)
{
	PatchSystemControl();
	InstallFlashEmu();

	previous = sctrlHENSetStartModuleHandler(OnModuleStart);

	ClearCaches();

	return 0;
}

//0x000002C4
int module_reboot_before(SceSize args, void *argp)
{
	UninstallFlashEmu();

	return 0;
}