/* 
 * Based on btcnf.h from minimum edition - https://github.com/PSP-Archive/minimum_edition
*/


#include <psptypes.h>

enum
{
	PATCH_ADD,
	PATCH_OVERWRITE,
	PATCH_OVERRIDE
};

typedef struct BtcnfHeader
{
	int signature;	  // 0
	int devkit;		  // 4
	int unknown[2];	  // 8
	int modestart;	  // 0x10
	int nmodes;		  // 0x14
	int unknown2[2];  // 0x18
	int modulestart;  // 0x20
	int nmodules;	  // 0x24
	int unknown3[2];  // 0x28
	int modnamestart; // 0x30
	int modnameend;	  // 0x34
	int unknown4[2];  // 0x38
} __attribute__((packed)) BtcnfHeader;

typedef struct ModuleEntry
{
	u32 stroffset; // 0
	int reserved;  // 4
	u16 flags;	   // 8
	u8 loadmode;   // 0x0a
	u8 loadmode2;  // 0x0B
	int reserved2; // 0x0C
	u8 hash[0x10]; // 0x10
} __attribute__((packed)) ModuleEntry;

typedef struct ModeEntry
{
	u16 maxsearch;
	u16 searchstart; //
	int modeflag;
	int mode2;
	int reserved[5];
} __attribute__((packed)) ModeEntry;

typedef struct ModuleList
{
	int patch_type;
	char *before_path;
	char *add_path;
	u16 flag;
	u16 loadmode;
} ModuleList;

ModuleList module_rtm[] = {
	{PATCH_ADD, "", "/rtm.prx", 0, 0x8001},
	{-1, NULL, NULL, 0, 0}};

static ModuleList *Get_list(ModuleList module_path[], const char *path)
{
	int i = 0;

	while (module_path[i].before_path)
	{
		if (strcmp(path, module_path[i].before_path) == 0)
		{
			module_path[i].before_path = "";
			return module_path + i;
		}
		i++;
	}

	return NULL;
}

void memcpy_b(void *dst, void *src, int len)
{
	u8 *d = (u8 *)dst;
	u8 *s = (u8 *)src;
	while(len--) 
	{
		d[len] = s[len];
	} 
}

int btcnf_patch(void *a0, int size, ModuleList patch_list[], int before, int after)
{
	ModuleList *list_stock;
	int ret = size; //
	int i, j;
	int module_cnt;

	BtcnfHeader *header = reinterpret_cast<BtcnfHeader *>(a0);

	if (header->signature == 0x0F803001)
	{
		module_cnt = header->nmodules;
		//		printf("module_cnt:0x%08X\n",module_cnt);
		if (module_cnt > 0)
		{

			ModuleEntry *module_offset = (ModuleEntry *)((u32)header + (u32)(header->modulestart));
			char *modname_start = (char *)((u32)header + header->modnamestart);

			for (i = 0; i < module_cnt; i++)
			{
				ModuleEntry *sp = &(module_offset[i]);

				if (before)
				{
					if (sp->flags & before)
					{
						sp->flags |= after;
					}
					else
					{
						sp->flags &= ~(after);
					}
				}

				if ((list_stock = Get_list(patch_list, modname_start + sp->stroffset)) != NULL)
				{
					//					printf("Add %s\n", list_stock->add_path );

					if (list_stock->patch_type == PATCH_OVERWRITE)
					{
						memcpy(modname_start + sp->stroffset, list_stock->add_path, strlen(list_stock->add_path) + 1);
					}
					else // if( list_stock->patch_type == PATCH_OVERRIDE || list_stock->patch_type == PATCH_ADD )
					{

						if (list_stock->patch_type == PATCH_ADD)
						{
							memcpy_b(&(module_offset[i + 1]), sp, ret - header->modulestart - 32 * i);
							ret += 32;
							header->nmodules++;
							header->modnamestart += 0x20;
							header->modnameend += 0x20;

							module_cnt++;

							int mode_cnt = header->nmodes;
							ModeEntry *mode_entyr = (ModeEntry *)((u32)header + (u32)(header->modestart));
							for (j = 0; j < mode_cnt; j++)
							{
								mode_entyr[j].maxsearch++;
								mode_entyr[j].searchstart = 0;
							}
						}
						ret += strlen(list_stock->add_path) + 1;
						sp->stroffset = header->modnameend - header->modnamestart;

						memcpy((char *)((u32)header + (u32)(header->modnameend)), list_stock->add_path, strlen(list_stock->add_path) + 1);
						header->modnameend += strlen(list_stock->add_path) + 1;
					}

					sp->flags = list_stock->flag; // flag
					sp->loadmode = list_stock->loadmode;

					modname_start = (char *)((u32)header + header->modnamestart);
				}
			}
		}
	}

	return ret;
}