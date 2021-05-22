#include <linux/cdev.h>   /* CDev definition */

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
