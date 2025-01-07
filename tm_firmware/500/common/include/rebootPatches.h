/* 
 * Based on payloadex_patch_addr.h/rebootex_patch_addr.h from minimum edition - https://github.com/PSP-Archive/minimum_edition
*/

#pragma once

#include <psptypes.h>

struct RebootPatches {
	u32 BootLfatMountPatch;
	u32 BootLfatOpenPatch;
	u32 BootLfatReadPatch;
	u32 BootLfatClosePatch;
	u32 CheckPspConfigPatch;
	u32 KdebugPatchAddr;
	u32 BtHeaderPatchAddr;
	u32 LfatMountPatchAddr;
	u32 LfatSeekPatchAddr1;
	u32 LfatSeekPatchAddr2;
	u32 LoadCorePatchAddr;
	u32 HashCheckPatchAddr;
	u32 SigcheckPatchAddr;
};

struct LoadCorePatches {
	u32 ModuleOffsetAddr;
	u32 SigcheckPatchAddr1;
	u32 SigcheckPatchAddr2;
	u32 SigcheckPatchAddr3;
	u32 SigcheckFuncAddr;
	u32 DecryptPatchAddr;
	u32 DecryptPatchAddr2;
	u32 DecryptFuncAddr;
};

struct Patches {
	struct RebootPatches rebootPatches;
	struct LoadCorePatches loadCorePatches;
};

struct MsLfatFuncs {
    void *msMount;
    void *msOpen;
    void *msRead;
    void *msClose;
};

static const struct Patches patches = {
#if PSP_MODEL == 0
#if defined PAYLOADEX
	.rebootPatches = {
		.BootLfatMountPatch	= 0x88603394,
      .BootLfatOpenPatch	= 0x886033a4,
		.BootLfatReadPatch	= 0x8860340c,
		.BootLfatClosePatch	= 0x8860342c,
		.CheckPspConfigPatch = 0x8860a308,
		.KdebugPatchAddr	   = 0x8860c1a0,
		.LfatMountPatchAddr	= 0x8860339c,
		.LfatSeekPatchAddr1	= 0x886033ec,
		.LfatSeekPatchAddr2	= 0x886033fc,
		.LoadCorePatchAddr	= 0x88602908,
		.HashCheckPatchAddr	= 0x88602e68,
		.SigcheckPatchAddr   = 0x886009c4,
	},
#elif defined REBOOTEX
	.rebootPatches = {
		.BootLfatMountPatch	= 0x88601f44,
      .BootLfatOpenPatch	= 0x88601f58,
		.BootLfatReadPatch	= 0x88601fc8,
		.BootLfatClosePatch	= 0x88601ff4,
		.CheckPspConfigPatch = 0x88604f68,
		.KdebugPatchAddr	   = 0x88603018,
		.LfatMountPatchAddr	= 0x88601f50,
		.LfatSeekPatchAddr1	= 0x88601fa4,
		.LfatSeekPatchAddr2	= 0x88601fbc,
		.LoadCorePatchAddr	= 0x88604e20,
		.HashCheckPatchAddr	= 0x88606c68,
		.SigcheckPatchAddr   = 0x8860133c,
	},
#endif

#elif (PSP_MODEL == 1)
#if defined PAYLOADEX
	.rebootPatches = {
      .BootLfatMountPatch  = 0x88603468,
		.BootLfatOpenPatch	= 0x88603478,
		.BootLfatReadPatch	= 0x886034e0,
		.BootLfatClosePatch	= 0x88603500,
		.CheckPspConfigPatch = 0x8860a3dc,
		.KdebugPatchAddr	   = 0x8860c274,
		.LfatMountPatchAddr	= 0x88603470,
		.LfatSeekPatchAddr1	= 0x886034c0,
		.LfatSeekPatchAddr2	= 0x886034d0,
		.LoadCorePatchAddr	= 0x886029d0,
		.HashCheckPatchAddr	= 0x88602f3c,
		.SigcheckPatchAddr   = 0x88600a54,
	},
#elif defined REBOOTEX
	.rebootPatches = {
      .BootLfatMountPatch	= 0x8860200c,
		.BootLfatOpenPatch	= 0x88602020,
		.BootLfatReadPatch	= 0x88602090,
		.BootLfatClosePatch	= 0x886020bc,
		.CheckPspConfigPatch = 0x88605030,
		.KdebugPatchAddr	   = 0x886030e0,
		.LfatMountPatchAddr	= 0x88602018,
		.LfatSeekPatchAddr1	= 0x8860206c,
		.LfatSeekPatchAddr2	= 0x88602084,
		.LoadCorePatchAddr	= 0x88604ee8,
		.HashCheckPatchAddr	= 0x88606d38,
		.SigcheckPatchAddr   = 0x886013cc,
	},
#endif

#endif
	.loadCorePatches = {
		.ModuleOffsetAddr	  = 0x00000c74,
		.SigcheckPatchAddr1 = 0x0000691c,
		.SigcheckPatchAddr2 = 0x0000694c,
		.SigcheckPatchAddr3 = 0x000069e4,
		.SigcheckFuncAddr	  = 0x000081b4,
		.DecryptPatchAddr	  = 0x000041d0,
		.DecryptPatchAddr2  = 0x000068f8,
		.DecryptFuncAddr	  = 0x000081d4,
	},
};

#ifdef PAYLOADEX
void patchIplPayload(MsLfatFuncs *funcs);
#elif defined REBOOTEX
void patchRebootBin(MsLfatFuncs *funcs);
#endif
