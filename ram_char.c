/* Disk on RAM Character Driver */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/genhd.h> 
#include <linux/blkdev.h>
#include <linux/hdreg.h> 
#include <linux/errno.h>

#include "ram_device.h"

#define RC_FIRST_MINOR 0
#define RC_MINOR_CNT 16

static u_int rc_major = 0;
static struct rc_device
{
	unsigned int size;
	spinlock_t lock;
	struct request_queue *rc_queue;
	struct gendisk *rc_disk;
} rc_dev;

static int rc_open(struct block_device *bdev, fmode_t mode)
{
	unsigned unit = iminor(bdev->bd_inode);

	printk(KERN_INFO "rc: Device is opened\n");
	printk(KERN_INFO "rc: Inode number is %d\n", unit);

	if (unit > RC_MINOR_CNT)
		return -ENODEV;
	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
static int rc_close(struct gendisk *disk, fmode_t mode)
{
	printk(KERN_INFO "rc: Device is closed\n");
	return 0;
}
#else
static void rc_close(struct gendisk *disk, fmode_t mode)
{
	printk(KERN_INFO "rc: Device is closed\n");
}
#endif

static int rc_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->heads = 1;
	geo->cylinders = 64;
	geo->sectors = 32;
	geo->start = 0;
	return 0;
}

static int rc_transfer(struct request *req)
{
	//struct rc_device *dev = (struct rc_device *)(req->rq_disk->private_data);

	int dir = rq_data_dir(req);
	sector_t start_sector = blk_rq_pos(req);
	unsigned int sector_cnt = blk_rq_sectors(req);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0))
#define BV_PAGE(bv) ((bv)->bv_page)
#define BV_OFFSET(bv) ((bv)->bv_offset)
#define BV_LEN(bv) ((bv)->bv_len)
	struct bio_vec *bv;
#else
#define BV_PAGE(bv) ((bv).bv_page)
#define BV_OFFSET(bv) ((bv).bv_offset)
#define BV_LEN(bv) ((bv).bv_len)
	struct bio_vec bv;
#endif
	struct req_iterator iter;

	sector_t sector_offset;
	unsigned int sectors;
	u8 *buffer;

	int ret = 0;

	printk(KERN_INFO "rc: Dir:%d; Sec:%lld; Cnt:%d\n", dir, start_sector, sector_cnt);

	sector_offset = 0;
	rq_for_each_segment(bv, req, iter)
	{
		buffer = page_address(BV_PAGE(bv)) + BV_OFFSET(bv);
		if (BV_LEN(bv) % RB_SECTOR_SIZE != 0)
		{
			printk(KERN_INFO "rc: Should never happen: "
				"bio size (%d) is not a multiple of RB_SECTOR_SIZE (%d).\n"
				"This may lead to data truncation.\n",
				BV_LEN(bv), RB_SECTOR_SIZE);
			ret = -EIO;
		}
		sectors = BV_LEN(bv) / RB_SECTOR_SIZE;
		printk(KERN_INFO "rc: Start Sector: %lld, Sector Offset: %lld; Buffer: %p; Length: %u sectors\n",start_sector, sector_offset, buffer, sectors);
		if (dir == WRITE) 
		{
			ramdevice_write(start_sector + sector_offset, buffer, sectors);
		}
		else
		{
			ramdevice_read(start_sector + sector_offset, buffer, sectors);
		}
		sector_offset += sectors;
	}
	if (sector_offset != sector_cnt)
	{
		printk(KERN_ERR "rc: bio info doesn't match with the request info");
		ret = -EIO;
	}

	return ret;
}
	
static void rc_request(struct request_queue *q)
{
	struct request *req;
	int ret;

	while ((req = blk_fetch_request(q)) != NULL)
	{
#if 0
		
		if (!blk_fs_request(req))
		{
			printk(KERN_NOTICE "rc: Skip non-fs request\n");
			__blk_end_request_all(req, 0);
			//__blk_end_request(req, 0, blk_rq_bytes(req));
			continue;
		}
#endif
		ret = rc_transfer(req);
		__blk_end_request_all(req, ret);
		//__blk_end_request(req, ret, blk_rq_bytes(req));
	}
}

static struct block_device_operations rc_fops =
{
	.owner = THIS_MODULE,
	.open = rc_open,
	.release = rc_close,
	.getgeo = rc_getgeo,
};
static int __init rc_init(void)
{
	int ret;
	if ((ret = ramdevice_init()) < 0)
	{
		return ret;
	}
	rc_dev.size = ret;

	rc_major = register_blkdev(rc_major, "rc");
	if (rc_major <= 0)
	{
		printk(KERN_ERR "rc: Unable to get Major Number\n");
		ramdevice_cleanup();
		return -EBUSY;
	}

	spin_lock_init(&rc_dev.lock);
	rc_dev.rc_queue = blk_init_queue(rc_request, &rc_dev.lock);
	if (rc_dev.rc_queue == NULL)
	{
		printk(KERN_ERR "rc: blk_init_queue failure\n");
		unregister_blkdev(rc_major, "rc");
		ramdevice_cleanup();
		return -ENOMEM;
	}
	
	rc_dev.rc_disk = alloc_disk(RC_MINOR_CNT);
	if (!rc_dev.rc_disk)
	{
		printk(KERN_ERR "rc: alloc_disk failure\n");
		blk_cleanup_queue(rc_dev.rc_queue);
		unregister_blkdev(rc_major, "rc");
		ramdevice_cleanup();
		return -ENOMEM;
	}

 	rc_dev.rc_disk->major = rc_major;
  	rc_dev.rc_disk->first_minor = RC_FIRST_MINOR;
 	rc_dev.rc_disk->fops = &rc_fops;
 	rc_dev.rc_disk->private_data = &rc_dev;
	rc_dev.rc_disk->queue = rc_dev.rc_queue;
	//rc_dev.rc_disk->flags = GENHD_FL_SUPPRESS_PARTITION_INFO;
	sprintf(rc_dev.rc_disk->disk_name, "rc");
	set_capacity(rc_dev.rc_disk, rc_dev.size);
	add_disk(rc_dev.rc_disk);
	printk(KERN_INFO "rc: Ram Block driver initialised (%d sectors; %d bytes)\n",
		rc_dev.size, rc_dev.size * RB_SECTOR_SIZE);

	return 0;
}
static void __exit rc_cleanup(void)
{
	del_gendisk(rc_dev.rc_disk);
	put_disk(rc_dev.rc_disk);
	blk_cleanup_queue(rc_dev.rc_queue);
	unregister_blkdev(rc_major, "rc");
	ramdevice_cleanup();
}

module_init(rc_init);
module_exit(rc_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ram Char Driver");
MODULE_ALIAS_BLOCKDEV_MAJOR(rc_major);
