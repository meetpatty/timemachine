#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <pspkernel.h>
#include <pspsysevent.h>
#include <psputilsforkernel.h>
#include <pspthreadman_kernel.h>
#include <pspsysmem_kernel.h>

#include <pspmacro.h>
#include <moduleUtils.h>
#include <flashemu.h>

#include <systemctrl.h>
#include <rebootex_01g.h>
#include <rebootex_02g.h>

PSP_MODULE_INFO("TimeMachine_Control", PSP_MODULE_KERNEL | PSP_MODULE_SINGLE_START | PSP_MODULE_SINGLE_LOAD | PSP_MODULE_NO_STOP, 1, 0);


STMOD_HANDLER previous;

void ClearCaches()
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheInvalidateAll();
}

int RebootBinDecompressPatched(u8 *dest, int destSize, u8 *src, int unk)
{
	if ((u32)dest == 0x88fc0000)
	{
		int model = sceKernelGetModel();
		if (model == 0)
			src = (char *)rebootex_01g;
		else if (model == 1 || model == 2)
			src = (char *)rebootex_02g;

		destSize = 0x10000;
	}

	return sceKernelGzipDecompress(dest, destSize, src, 0);
}

int codePagesceIoOpenPatched(const char *file, int flags, SceMode mode)
{
	SceModule2 *mod = (SceModule2 *)sceKernelFindModuleByName("vsh_module");

	if (!mod)
		return 0x80010018;

	return sceIoOpen(file, flags, mode);
}

int OnModuleStart(SceModule2 *mod)
{
	char *moduleName = mod->modname;
	u32 text_addr = mod->text_addr;

	if (strcmp(moduleName, "sceUtility_Driver") == 0)
	{
		SceModule2 *mod2 = (SceModule2 *)sceKernelFindModuleByName("sceMSFAT_Driver");

		MAKE_CALL(mod2->text_addr + 0x30fc, df_openPatched);
		MAKE_CALL(mod2->text_addr + 0x3ba4, df_dopenPatched);
		MAKE_CALL(mod2->text_addr + 0x44cc, df_devctlPatched);

		ClearCaches();
	}
	else if (strcmp(moduleName, "sceLflashFatfmt") == 0)
	{
		u32 funcAddr = sctrlHENFindFunction("sceLflashFatfmt", "LflashFatfmt", 0xb7a424a4); // sceLflashFatfmtStartFatfmt
		if (funcAddr)
		{
			MAKE_FUNCTION_RETURN0(funcAddr);
			ClearCaches();
		}
	}
	else if (strcmp(moduleName, "sceCodepage_Service") == 0)
	{
		u32 stubAddr = GetModuleImportFuncAddr("sceCodepage_Service", "IoFileMgrForKernel", 0x109f50bc); // sceIoOpen
		if (stubAddr)
			REDIRECT_FUNCTION(stubAddr, codePagesceIoOpenPatched);

		ClearCaches();
	}

	if (!previous)
		return 0;

	return previous(mod);
}

void PatchSystemControl()
{
	// Patch import stub of sceKernelGzipDecompress
	MAKE_JUMP((u32)GetModuleImportFuncAddr("SystemControl", "UtilsForKernel", 0x78934841), RebootBinDecompressPatched);
}

void PatchLoadCore()
{
	SceModule2 *mod = (SceModule2 *)sceKernelFindModuleByName("sceLoaderCore");

	//Restore nand decrypt function call
	MAKE_CALL(mod->text_addr + 0x5994, mod->text_addr + 0x7824);
}

int module_start(SceSize args, void *argp)
{
	PatchSystemControl();
	InstallFlashEmu();
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);
	PatchLoadCore();
	ClearCaches();

	return 0;
}

int module_reboot_before(SceSize args, void *argp)
{
	UninstallFlashEmu();

	return 0;
}
