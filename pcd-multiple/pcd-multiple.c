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
