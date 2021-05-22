#include <linux/cdev.h>   /* CDev definition */
#include <linux/fs.h>     /* File Operations */
#include <linux/uaccess.h> /* copy_to/from_user */

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

static pcd_t pcd;

static int pcd_init(pcd_t *pcd);
int	pcd_open(struct inode *inode, struct file *file);
int	pcd_release(struct inode *inode, struct file *file);
ssize_t pcd_read(struct file *file, char __user *buff, size_t count, loff_t *f_pos);
ssize_t pcd_write(struct file *file, const char __user *buff, size_t count, loff_t *f_pos);
loff_t 	pcd_lseek(struct file *file, loff_t offset, int whence);

static int pcd_check_permission(int dev_perm, int access_mode);

static int pcd_init(pcd_t *pcd){
	if(pcd == NULL){
		return (-1);
	}

	pcd->driver_data.total_devices = 			PCD_NUM_OF_DEVICES;
	/*
	 * Setting 1 device
	 */
	pcd->driver_data.devices[0].buffer = 		pcd->pcd_1_buffer;
	pcd->driver_data.devices[0].size = 			MEM_SIZE_PCD_1;
	pcd->driver_data.devices[0].serial_number = PCD_1_SERIAL_NUM;
	pcd->driver_data.devices[0].perm =			PCD_PERM_RDONLY;
	/*
	 * Setting 2 device
	 */
	pcd->driver_data.devices[1].buffer = 		pcd->pcd_2_buffer;
	pcd->driver_data.devices[1].size = 			MEM_SIZE_PCD_2;
	pcd->driver_data.devices[1].serial_number = PCD_2_SERIAL_NUM;
	pcd->driver_data.devices[1].perm =			PCD_PERM_WRONLY;
	/*
	 * Setting 3 device
	 */
	pcd->driver_data.devices[2].buffer = 		pcd->pcd_3_buffer;
	pcd->driver_data.devices[2].size = 			MEM_SIZE_PCD_3;
	pcd->driver_data.devices[2].serial_number = PCD_3_SERIAL_NUM;
	pcd->driver_data.devices[2].perm =			PCD_PERM_RDWR;
	/*
	 * Setting 4 device
	 */
	pcd->driver_data.devices[3].buffer = 		pcd->pcd_4_buffer;
	pcd->driver_data.devices[3].size = 			MEM_SIZE_PCD_4;
	pcd->driver_data.devices[3].serial_number = PCD_4_SERIAL_NUM;
	pcd->driver_data.devices[3].perm =			PCD_PERM_RDWR;
	

	pcd->fops.open = pcd_open;
	pcd->fops.release = pcd_release;
	pcd->fops.read = pcd_read;
	pcd->fops.write = pcd_write;
	pcd->fops.llseek = pcd_lseek;
	pcd->fops.owner = THIS_MODULE;

	return 0;
}

/*
 * Steps:
 * 1. Find out on which device file open was attemped by the user-space
 * 2. Get Device's private data
 * 3. Check Permission
 */
int pcd_open(struct inode *inode, struct file *file)
{
	int rc = (-1);
	int minor_num = 0;
	struct device_private_data *dev_priv_data;

	/* Find out to which device is userspace trying to get the access */
	minor_num = MINOR(inode->i_rdev);
	pr_info("minor number is %d\n", minor_num);

	/* Get Device's private data */
	dev_priv_data =
		container_of(inode->i_cdev, struct device_private_data, cdev);

	/* Supply device private data to other driver's methods */
	file->private_data = dev_priv_data;

	rc = pcd_check_permission(dev_priv_data->perm, file->f_mode);
	(!rc) ? pr_info("Success\n") : pr_info("Unsuccess\n");

	return rc;
}

int pcd_release(struct inode *inode, struct file *file)
{
	pr_info("Success\n");
	return 0;
}

/*
 * Steps:
 * 1. Check 'count' against buffer_size
 *   - if f_pos(current_file_pos) + 'count' > buffer_size --> 'count' = buffer_size - f_pos
 * 2. Copy 'count' bytes from pcd's buffer to the user's buffer
 * 3. Update f_pos
 * 4. Return number of bytes successfully read or error code
 * 5. If f_pos at EOF then return 0
 */
ssize_t pcd_read(struct file *file, char __user *buff, size_t count,
		 loff_t *f_pos)
{
	pr_info("Request %zu bytes\n", count);
	pr_info("Current file position %lld\n", (*f_pos));

	struct device_private_data *dev_priv_data = (struct device_private_data *)file->private_data;
	int buffer_size = dev_priv_data->size;

	/* Step 1 */
	if ((*f_pos + count) > buffer_size) {
		count = buffer_size - (*f_pos);
	}

	/* Step 2 */
	/* TODO: access to global variable should be serialized */
	if (copy_to_user(buff, dev_priv_data->buffer + (*f_pos), count)) {
		return -EFAULT;
	}

	/* Step 3 */
	*f_pos += count;

	pr_info("Successfully read %zu bytes\n", count);
	pr_info("Update file position %lld\n", (*f_pos));

	/* Step 4 */
	return count;
}

/*
 * Steps:
 * 1. Check 'count' against buffer_size
 *   - if f_pos(current_file_pos) + 'count' > buffer_size --> 'count' = buffer_size - f_pos
 * 2. Copy 'count' bytes from user's buffer to  pcd's buffer
 * 3. Update f_pos (current_file_pos)
 * 4. Return number of bytes successfully read or error code
 */
ssize_t pcd_write(struct file *file, const char __user *buff, size_t count,
		  loff_t *f_pos)
{
	pr_info("Request %zu bytes\n", count);
	pr_info("Current file position %lld\n", (*f_pos));

	struct device_private_data *dev_priv_data = (struct device_private_data *)file->private_data;
	int buffer_size = dev_priv_data->size;

	/* Step 1 */
	if ((*f_pos + count) > buffer_size) {
		count = buffer_size - (*f_pos);
	}
	if (!count) {
        pr_info("There is no space on the device!\n");
		return -ENOMEM;
	}

	/* Step 2 */
	/* TODO: access to global variable should be serialized */
	if (copy_from_user(dev_priv_data->buffer + (*f_pos), buff, count)) {
		return -EFAULT;
	}

	/* Step 3 */
	(*f_pos) += count;

	pr_info("Successfully write %zu bytes\n", count);
	pr_info("Update file position %lld\n", (*f_pos));

	return count;
}

/*
 * Whence can be {SEEK_SET, SEEK_CUR, SEEK_END}
 *
 * Steps:
 * 1. Check 'count' against buffer_size
 *   - if f_pos(current_file_pos) + 'count' > buffer_size --> 'count' = buffer_size - f_pos
 * 2. Copy 'count' bytes from user's buffer to  pcd's buffer
 * 3. Update f_pos (current_file_pos)
 * 4. Return number of bytes successfully read or error code
 */
loff_t pcd_lseek(struct file *file, loff_t offset, int whence)
{
	pr_info("Request\n");
	pr_info("Current file position %lld\n", (file->f_pos));

	loff_t temp = (-1);
	struct device_private_data *dev_priv_data = (struct device_private_data *)file->private_data;
	int buffer_size = dev_priv_data->size;

	switch (whence) {
	case SEEK_SET:
		if ((offset > buffer_size) || (offset < 0)) {
			return -EINVAL;
		}
		file->f_pos = offset;
		break;
	case SEEK_CUR:
		temp = file->f_pos + offset;
		if ((temp > buffer_size) || (temp < 0)) {
			return -EINVAL;
		}
		file->f_pos = temp;
		break;
	case SEEK_END:
		temp = buffer_size + offset;
		if ((temp > buffer_size) || (temp < 0)) {
			return -EINVAL;
		}
		file->f_pos = temp;
		break;
	default:
		return -EINVAL;
	}

	pr_info("Success\n");
	pr_info("Update file position %lld\n", (file->f_pos));

	return file->f_pos;
}

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
