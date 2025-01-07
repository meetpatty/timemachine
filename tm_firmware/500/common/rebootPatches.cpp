/* 
 * Based on code from minimum edition - https://github.com/PSP-Archive/minimum_edition
*/

#include <type_traits>
#include <string.h>

#include <pspmacro.h>
#include <cache.h>

#include "rebootPatches.h"

#if PSP_MODEL == 0
#define OE_TAG 0xa030db35
#else
#define OE_TAG 0xb301aeba
#endif


namespace {

    using nand_dec_function_t = std::add_pointer_t<int(u8*, u32)>;
    using decrypt_function_t = std::add_pointer_t<int(u8*, u32, s32*)>;
    using module_start_function_t = std::add_pointer_t<int(SceSize, void*)>;

    nand_dec_function_t nandDecryptPtr;
    decrypt_function_t decryptPtr;
    u32 loadCoreTextAddr;

    void clearCaches() {
		iplKernelDcacheWritebackInvalidateAll();
		iplKernelIcacheInvalidateAll();
	}

    int decryptPatched(u8 *buf, u32 size, s32 *s) {

        u32 tag = *reinterpret_cast<u32 *>(buf + 0x130);
        s32 compSize = *reinterpret_cast<s32 *>(buf + 0xb0);
        u32 *data = reinterpret_cast<u32 *>(buf + 0x150);

        if (tag == OE_TAG)
        {
            *s = compSize;

			if (compSize > 0) {
				memcpy(buf, data, compSize);
			}

            return 0;
        }

        return decryptPtr(buf, size, s);
    }

    int loadCoreModuleStartPatched(SceSize args, void *argp, module_start_function_t module_start) {

        loadCoreTextAddr = reinterpret_cast<u32>(module_start) - patches.loadCorePatches.ModuleOffsetAddr;

        _sw(ADDIU(GPREG_V0, GPREG_ZR, 0), loadCoreTextAddr + patches.loadCorePatches.SigcheckPatchAddr1);
        _sw(ADDIU(GPREG_V0, GPREG_ZR, 0), loadCoreTextAddr + patches.loadCorePatches.SigcheckPatchAddr2);
        _sw(ADDIU(GPREG_V0, GPREG_ZR, 0), loadCoreTextAddr + patches.loadCorePatches.SigcheckPatchAddr3);

        MAKE_CALL(loadCoreTextAddr + patches.loadCorePatches.DecryptPatchAddr, decryptPatched);
        MAKE_CALL(loadCoreTextAddr + patches.loadCorePatches.DecryptPatchAddr2, decryptPatched);

        nandDecryptPtr = reinterpret_cast<nand_dec_function_t>(loadCoreTextAddr + patches.loadCorePatches.SigcheckFuncAddr);
        decryptPtr = reinterpret_cast<decrypt_function_t>(loadCoreTextAddr + patches.loadCorePatches.DecryptFuncAddr);

		clearCaches();

		return module_start(8, argp);
	}
}

#if defined PAYLOADEX
void patchIplPayload(MsLfatFuncs *funcs) {

    _sw(ADDIU(GPREG_A2, GPREG_T7, 0x0), patches.rebootPatches.LoadCorePatchAddr);

#elif defined REBOOTEX
void patchRebootBin(MsLfatFuncs *funcs) {

    _sw(ADDIU(GPREG_A2, GPREG_S1, 0x0), patches.rebootPatches.LoadCorePatchAddr);
#endif

    MAKE_JUMP(patches.rebootPatches.LoadCorePatchAddr + 0x8, loadCoreModuleStartPatched);

	_sw(SW(GPREG_A1, GPREG_SP, 0x0), patches.rebootPatches.CheckPspConfigPatch);
	_sw(ADDI(GPREG_V1, GPREG_A1, 0x0), patches.rebootPatches.CheckPspConfigPatch + 0x4);

	MAKE_FUNCTION_RETURN0(patches.rebootPatches.SigcheckPatchAddr);

	_sw(0, patches.rebootPatches.LfatMountPatchAddr);
	_sw(0, patches.rebootPatches.LfatSeekPatchAddr1);
	_sw(0, patches.rebootPatches.LfatSeekPatchAddr2);
	_sw(0, patches.rebootPatches.HashCheckPatchAddr);

	MAKE_FUNCTION_RETURN(patches.rebootPatches.KdebugPatchAddr, 1);

	MAKE_CALL(patches.rebootPatches.BootLfatMountPatch, funcs->msMount);
	MAKE_CALL(patches.rebootPatches.BootLfatOpenPatch, funcs->msOpen);
	MAKE_CALL(patches.rebootPatches.BootLfatReadPatch, funcs->msRead);
	MAKE_CALL(patches.rebootPatches.BootLfatClosePatch, funcs->msClose);

    clearCaches();
}
