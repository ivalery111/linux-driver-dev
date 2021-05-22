#include <linux/cdev.h>   /* CDev definition */
#include <linux/fs.h>     /* File Operations */

/*
 * Device Private Data
 */
struct device_private_data {
	char *buffer;
	unsigned size;

#define PCD_1_SERIAL_NUM ("PCDXYZ_1_Q")
#define PCD_2_SERIAL_NUM ("PCDXYZ_2_Q")
#define PCD_3_SERIAL_NUM ("PCDXYZ_3_Q")
#define PCD_4_SERIAL_NUM ("PCDXYZ_4_Q")
	const char *serial_number;

#define PCD_PERM_RDONLY (0x01)
#define PCD_PERM_WRONLY (0x10)
#define PCD_PERM_RDWR	(0x11)
	int perm;

	struct cdev cdev; /* Character device */
};

/*
 * Driver Private Data
 */
struct driver_private_data {
#define PCD_MAJOR_NUM 0
#define PCD_MINOR_NUM 1
#define PCD_DEVICES_NAME "pcd"
	dev_t dev_number; /* Holds the device number */

#define PCD_CLASS_NAME  "pcd_class"
	struct class *class;

#define PCD_DEVICE_NAME "pcd"
	struct device *device;

#define PCD_NUM_OF_DEVICES (4)
	int total_devices;
	struct device_private_data devices[PCD_NUM_OF_DEVICES];
};

/*
 * Device definition
 */
typedef struct pcd_s {
#define MEM_SIZE_PCD_1 512
#define MEM_SIZE_PCD_2 512
#define MEM_SIZE_PCD_3 512
#define MEM_SIZE_PCD_4 512
	char pcd_1_buffer[MEM_SIZE_PCD_1];
	char pcd_2_buffer[MEM_SIZE_PCD_2];
	char pcd_3_buffer[MEM_SIZE_PCD_3];
	char pcd_4_buffer[MEM_SIZE_PCD_4];

	struct driver_private_data driver_data;
	struct device_private_data device_data;

	struct file_operations fops; /* File Operations */
} pcd_t;

static int pcd_check_permission(int dev_perm, int access_mode);

static int pcd_check_permission(int dev_perm, int access_mode){

	if (dev_perm == (PCD_PERM_RDWR)) {
		return 0;
	}
	if ((dev_perm == PCD_PERM_RDONLY) && (access_mode & FMODE_READ)
	    && !(access_mode & FMODE_WRITE)) {
		return 0;
	}
	if ((dev_perm == PCD_PERM_WRONLY) && (access_mode & FMODE_WRITE)
	    && !(access_mode & FMODE_READ)) {
		return 0;
	}

	return -EPERM;
}
