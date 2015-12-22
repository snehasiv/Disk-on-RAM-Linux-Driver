#include <linux/string.h>

#include "partition.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

#define SECTOR_SIZE 512
#define MBR_SIZE SECTOR_SIZE
#define MBR_DISK_SIGNATURE_OFFSET 442
#define MBR_DISK_SIGNATURE_SIZE 4
#define PARTITION_TABLE_OFFSET 446
#define PARTITION_ENTRY_SIZE 16 
#define PARTITION_TABLE_SIZE 64 
#define MBR_SIGNATURE_OFFSET 510
#define MBR_SIGNATURE_SIZE 2
#define MBR_SIGNATURE 0xAA55
#define BR_SIZE SECTOR_SIZE
#define BR_SIGNATURE_OFFSET 510
#define BR_SIGNATURE_SIZE 2
#define BR_SIGNATURE 0xAA55

typedef struct
{
	unsigned char bootType; 
	unsigned char startHead;
	unsigned char startSec:6;
	unsigned char startCylH:2;
	unsigned char startCyl;
	unsigned char partType;
	unsigned char endHead;
	unsigned char endSec:6;
	unsigned char endCylH:2;
	unsigned char endCyl;
	unsigned int AbsStartSec;
	unsigned int secCtr;
} PartEntry;

typedef PartEntry PartTable[4];

static PartTable def_part_table =
{
	{
		bootType: 0x00,
		startHead: 0x00,
		startSec: 0x2,
		startCyl: 0x00,
		partType: 0x83,
		endHead: 0x00,
		endSec: 0x20,
		endCyl: 0x0E,
		AbsStartSec: 0x00000001,
		secCtr: 0x000001DF
	},
	{
		bootType: 0x00,
		startHead: 0x00,
		startSec: 0x1,
		startCyl: 0x0F, 
		partType: 0x05,
		endHead: 0x00,
		endSec: 0x20,
		endCyl: 0x1D,
		AbsStartSec: 0x000001E0,
		secCtr: 0x000001E0
	},
	{
		bootType: 0x00,
		startHead: 0x00,
		startSec: 0x1,
		startCyl: 0x1E,
		partType: 0x83,
		endHead: 0x00,
		endSec: 0x20,
		endCyl: 0x30,
		AbsStartSec: 0x000003C0,
		secCtr: 0x00000260
	},
	{
		
		bootType: 0x00,
		startHead: 0x00,
		startSec: 0x1,
		startCyl: 0x31,
		partType: 0x83,
		endHead: 0x00,
		endSec: 0x20,
		endCyl: 0x3F,
		AbsStartSec: 0x00000620,
		secCtr: 0x000001E0
	}
};
static unsigned int def_log_part_br_cyl[] = {0x0F, 0x14, 0x19};
static const PartTable def_log_part_table[] =
{
	{
		{
			bootType: 0x00,
			startHead: 0x00,
			startSec: 0x2,
			startCyl: 0x0F,
			partType: 0x83,
			endHead: 0x00,
			endSec: 0x20,
			endCyl: 0x13,
			AbsStartSec: 0x00000001,
			secCtr: 0x0000009F
		},
		{
			bootType: 0x00,
			startHead: 0x00,
			startSec: 0x1,
			startCyl: 0x14,
			partType: 0x05,
			endHead: 0x00,
			endSec: 0x20,
			endCyl: 0x18,
			AbsStartSec: 0x000000A0,
			secCtr: 0x000000A0
		},
	},
	{
		{
			bootType: 0x00,
			startHead: 0x00,
			startSec: 0x2,
			startCyl: 0x14,
			partType: 0x83,
			endHead: 0x00,
			endSec: 0x20,
			endCyl: 0x18,
			AbsStartSec: 0x00000001,
			secCtr: 0x0000009F
		},
		{
			bootType: 0x00,
			startHead: 0x00,
			startSec: 0x1,
			startCyl: 0x19,
			partType: 0x05,
			endHead: 0x00,
			endSec: 0x20,
			endCyl: 0x1D,
			AbsStartSec: 0x00000140,
			secCtr: 0x000000A0
		},
	},
	{
		{
			bootType: 0x00,
			startHead: 0x00,
			startSec: 0x2,
			startCyl: 0x19,
			partType: 0x83,
			endHead: 0x00,
			endSec: 0x20,
			endCyl: 0x1D,
			AbsStartSec: 0x00000001,
			secCtr: 0x0000009F
		},
	}
};

static void copyMbr(u8 *disk)
{
	memset(disk, 0x0, MBR_SIZE);
	*(unsigned long *)(disk + MBR_DISK_SIGNATURE_OFFSET) = 0x36E5756D;
	memcpy(disk + PARTITION_TABLE_OFFSET, &def_part_table, PARTITION_TABLE_SIZE);
	*(unsigned short *)(disk + MBR_SIGNATURE_OFFSET) = MBR_SIGNATURE;
}
static void copyBr(u8 *disk, int startCylinder, const PartTable *part_table)
{
	disk += (startCylinder * 32 * SECTOR_SIZE);
	memset(disk, 0x0, BR_SIZE);
	memcpy(disk + PARTITION_TABLE_OFFSET, part_table,
		PARTITION_TABLE_SIZE);
	*(unsigned short *)(disk + BR_SIGNATURE_OFFSET) = BR_SIGNATURE;
}
void copyMbrBr(u8 *disk)
{
	int i;

	copyMbr(disk);
	for (i = 0; i < ARRAY_SIZE(def_log_part_table); i++)
	{
		copyBr(disk, def_log_part_br_cyl[i], &def_log_part_table[i]);
	}
}
