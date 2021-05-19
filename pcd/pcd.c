#include <linux/module.h>
#include <linux/kdev_t.h> /* dev_t */
#include <linux/fs.h>	  /* Device number */
#include <linux/cdev.h>   /* CDev definition */
#include <linux/fs.h>     /* File Operations */
#include <linux/device.h> /* Device files */

/* Customise the print message */
#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt, __func__

typedef struct pcd_s {
#define PCD_MEM_SIZE 512
	char buffer[PCD_MEM_SIZE];

#define PCD_MAJOR_NUM    0
#define PCD_MINOR_NUM    1
#define PCD_DEVICES_NAME "pcd"
	dev_t dev_number;            /* Holds the device number */

    struct cdev cdev;            /* Character device */

    struct file_operations fops; /* File Operations */

#define PCD_CLASS_NAME  "pcd_class"
#define PCD_DEVICE_NAME "pcd"
    struct class *class;
    struct device *device;
} pcd_t;

int pcd_open(struct inode *inode, struct file *file);
int pcd_release(struct inode *inode, struct file *file);
ssize_t pcd_read(struct file *file, char __user *buff, size_t count, loff_t *f_pos);
ssize_t pcd_write(struct file *file, const char __user *buff, size_t count, loff_t *f_pos);
loff_t pcd_lseek(struct file *file, loff_t offset, int whence);

static pcd_t pcd = {
		.fops.open    = pcd_open,
		.fops.release = pcd_release,
		.fops.read    = pcd_read,
		.fops.write   = pcd_write,
		.fops.llseek  = pcd_lseek,
		.fops.owner   = THIS_MODULE,
	};

static int __init pcd_driver_init(void)
{
	int rc = (-1);

	/*
	 * 1. Dynamically allocate a device number
	 */
	rc = alloc_chrdev_region(&pcd.dev_number,
                              PCD_MAJOR_NUM,
                              PCD_MINOR_NUM,
			                  PCD_DEVICES_NAME);
	if (rc < 0) {
		pr_err("Alloc chrdev failed!\n");
		goto exit;
	}
    pr_info("Device Number <major>:<minor> -> %d:%d\n",
                                        MAJOR(pcd.dev_number),
                                        MINOR(pcd.dev_number));
    /*
     * 2. Character Device Registration
     */
    /* 2.1 Initialize the cdev with file operations */
    cdev_init(&pcd.cdev, &pcd.fops);

    /* 2.2 Register the device with VFS */
    pcd.cdev.owner = THIS_MODULE;
    rc = cdev_add(&pcd.cdev, pcd.dev_number, PCD_MINOR_NUM);
    if (rc < 0) {
	    pr_err("Cdev add failed!\n");
	    goto unreg_chrdev;
    }

    /*
     * 3. Creating the device class under /sys/class/
     */
    pcd.class = class_create(THIS_MODULE, PCD_CLASS_NAME);
    if (IS_ERR(pcd.class)) {
	    pr_err("Class creation failed!\n");
	    rc = PTR_ERR(pcd.class);
	    goto cdev_del;
    }

    /*
     * 3.1 Populate the sysfs with device information
     */
    pcd.device = device_create(pcd.class, NULL, pcd.dev_number, NULL, PCD_DEVICE_NAME);
    if (IS_ERR(pcd.device)) {
	    pr_err("Device creation failed!\n");
	    rc = PTR_ERR(pcd.device);
	    goto class_destroy;
    }

    pr_info("Module init successful!\n");

    return 0;

class_destroy:
	class_destroy(pcd.class);
cdev_del:
	cdev_del(&pcd.cdev);
unreg_chrdev:
	unregister_chrdev_region(pcd.dev_number, PCD_MINOR_NUM);
exit:
	pr_err("Module insertion failed!\n");
	return rc;
}

static void __exit pcd_driver_cleanup(void)
{
    device_destroy(pcd.class, pcd.dev_number);
    class_destroy(pcd.class);
    cdev_del(&pcd.cdev);
    unregister_chrdev_region(pcd.dev_number, PCD_MINOR_NUM);

    pr_info("Module unloaded!\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valery Ivanov");
MODULE_DESCRIPTION("A Pseudo Character Driver");

int pcd_open(struct inode *inode, struct file *file)
{
    pr_info("Success\n");
	return 0;
}
int pcd_release(struct inode *inode, struct file *file)
{
    pr_info("Success\n");
	return 0;
}

/*
 * Steps:
 * 1. Check 'count' against PCD_MEM_SIZE
 *   - if f_pos(current_file_pos) + 'count' > PCD_MEM_SIZE --> 'count' = PCD_MEM_SIZE - f_pos
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

	/* Step 1 */
	if ((*f_pos + count) > PCD_MEM_SIZE) {
		count = PCD_MEM_SIZE - (*f_pos);
	}

	/* Step 2 */
	/* TODO: access to global variable should be serialized */
	if (copy_to_user(buff, &pcd.buffer[(*f_pos)], count)) {
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
 * 1. Check 'count' against PCD_MEM_SIZE
 *   - if f_pos(current_file_pos) + 'count' > PCD_MEM_SIZE --> 'count' = PCD_MEM_SIZE - f_pos
 * 2. Copy 'count' bytes from user's buffer to  pcd's buffer
 * 3. Update f_pos (current_file_pos)
 * 4. Return number of bytes successfully read or error code
 */
ssize_t pcd_write(struct file *file, const char __user *buff, size_t count,
		  loff_t *f_pos)
{
	pr_info("Request %zu bytes\n", count);
	pr_info("Current file position %lld\n", (*f_pos));

	/* Step 1 */
	if ((*f_pos + count) > PCD_MEM_SIZE) {
		count = PCD_MEM_SIZE - (*f_pos);
	}
	if (!count) {
		return -ENOMEM;
	}

	/* Step 2 */
	/* TODO: access to global variable should be serialized */
	if (copy_from_user(&pcd.buffer[(*f_pos)], buff, count)) {
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
 * 1. Check 'count' against PCD_MEM_SIZE
 *   - if f_pos(current_file_pos) + 'count' > PCD_MEM_SIZE --> 'count' = PCD_MEM_SIZE - f_pos
 * 2. Copy 'count' bytes from user's buffer to  pcd's buffer
 * 3. Update f_pos (current_file_pos)
 * 4. Return number of bytes successfully read or error code
 */
loff_t pcd_lseek(struct file *file, loff_t offset, int whence)
{
	loff_t temp = (-1);

	pr_info("Request\n");
	pr_info("Current file position %lld\n", (file->f_pos));

	switch (whence) {
	case SEEK_SET:
		if ((offset > PCD_MEM_SIZE) || (offset < 0)) {
			return -EINVAL;
		}
		file->f_pos = offset;
		break;
	case SEEK_CUR:
		temp = file->f_pos + offset;
		if ((temp > PCD_MEM_SIZE) || (temp < 0)) {
			return -EINVAL;
		}
		file->f_pos = temp;
		break;
	case SEEK_END:
		temp = PCD_MEM_SIZE + offset;
		if ((temp > PCD_MEM_SIZE) || (temp < 0)) {
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
