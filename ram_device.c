#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/errno.h>

#include "ram_device.h"
#include "partition.h"

#define RB_DEVICE_SIZE 2048 /*sectors*/
/* So, total device size = 2048 * 512 bytes = 1MB */

static u8 *d_data;

int ramdevice_init(void)
{
	d_data = vmalloc(RB_DEVICE_SIZE * RB_SECTOR_SIZE);
	if (d_data == NULL)
		return -ENOMEM;
	copyMbrBr(d_data);
	return RB_DEVICE_SIZE;
}

void ramdevice_cleanup(void)
{
	vfree(d_data);
}

void ramdevice_write(sector_t sector_off, u8 *buf, unsigned int sectors)
{
	memcpy(d_data + sector_off * RB_SECTOR_SIZE, buf,
		sectors * RB_SECTOR_SIZE);
}

void ramdevice_read(sector_t sector_off, u8 *buf, unsigned int sectors)
{
	memcpy(buf, d_data + sector_off * RB_SECTOR_SIZE,
		sectors * RB_SECTOR_SIZE);
}

