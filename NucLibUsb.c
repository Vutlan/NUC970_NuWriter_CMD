
#include "common.h"

int NUC_SetType(int id,int type)
{
	unsigned char ack[2] = {0, 0};
	int res=0;

	/* Waiting the last work done before switching */
	usleep(90 * 1000u);
	do {
		usleep(10 * 1000u);
		res = libusb_control_transfer(handle,
		                        0x40, /* requesttype */
		                        BURN, /* request */
		                        BURN_TYPE+(unsigned int)type, /* wValue */
		                        0, /* wIndex */
		                        NULL,
		                        0, /* wLength */
		                        USB_TIMEOUT);
		if (res < 0) {
			MSG_DEBUG("[%s:%d] %s\n",__FUNCTION__, __LINE__, libusb_error_name(res));
			return res;
		}
		res = libusb_control_transfer(handle,
		                        0xC0, /* requesttype */
		                        BURN, /* request */
		                        BURN_TYPE+(unsigned int)type, /* wValue */
		                        0, /* wIndex */
		                        (unsigned char *)&ack,
		                        (uint16_t)sizeof(ack), /* wLength */
		                        USB_TIMEOUT);
		if (res < 0) {
			MSG_DEBUG("[%s:%d] %s\n",__FUNCTION__, __LINE__,libusb_error_name(res));
			return res;
		}
	}
    while(ack[0] != (BURN_TYPE + type));
	return 0;
}


int NUC_ReadPipe(int id,unsigned char *buf,int len)
{
	int nread, ret;

	ret = libusb_bulk_transfer(handle, USB_ENDPOINT_IN, buf, len,
	                           &nread, USB_TIMEOUT);
	if (ret) {
		MSG_DEBUG("ERROR in bulk read: %s\n", libusb_error_name(ret));
		return -1;
	} else {
		//MSG_DEBUG("receive %d bytes from device\n", nread);
		//printf("%s", receiveBuf);  //Use this for benchmarking purposes
		return 0;
	}
}
int NUC_WritePipe(int id,unsigned char *buf,int len)
{

	int ret;
	int nwrite;

	ret = libusb_control_transfer(handle,
	                        0x40, /* requesttype */
	                        0xA0, /* request */
	                        0x12, /* wValue */
	                        len, /* wIndex */
	                        buf,
	                        0, /* wLength */
	                        USB_TIMEOUT);
	if (ret < 0) {
			MSG_DEBUG("[%s:%d] %s\n",__FUNCTION__, __LINE__, libusb_error_name(ret));
			return ret;
	}

	//write transfer
	//probably unsafe to use n twice...
	ret = libusb_bulk_transfer(handle, USB_ENDPOINT_OUT, buf, len, &nwrite, USB_TIMEOUT);
	//Error handling
	if (ret < 0)
		printf("ERROR in bulk write: %s\n", libusb_error_name(ret));

	return ret;
}

int NUC_IsDeviceConnectedToBP(int bus, int port)
{
	int ret=0, found=0;
	uint8_t ibus, iport;
	ssize_t i;

	struct libusb_device **devs;
	ssize_t count;
	count = libusb_get_device_list(NULL, &devs);
	for (i = 0; i < count; i++) {
		struct libusb_device_descriptor desc;
		//memset(&desc, 0, sizeof(desc));
		ret = libusb_get_device_descriptor(devs[i], &desc);
		if (ret !=0)
			continue;

		if ((desc.idVendor == USB_VENDOR_ID) &&	(desc.idProduct == USB_PRODUCT_ID)) {
			ibus = libusb_get_bus_number(devs[i]);
			iport = libusb_get_port_number(devs[i]);
			if ((bus == ibus) && (port == iport)) {
				found=1;
				break;
			}
		}
	}
	libusb_free_device_list(devs, 0);
	return found;
}

static int NUC_OpenUsb_priv(int *bus, int *port, int suppres_nodev)
{
	int ret=0;
	uint8_t ibus, iport;
	ssize_t i;
	struct libusb_device **devs;
	ssize_t count;

	if (handle != NULL) return 0;

	count = libusb_get_device_list(NULL, &devs);
	for (i = 0; i < count; i++) {
		struct libusb_device_descriptor desc;
		memset(&desc, 0, sizeof(desc));

		ret = libusb_get_device_descriptor(devs[i], &desc);
		if (ret !=0)
			continue;

		if ((desc.idVendor == USB_VENDOR_ID) &&	(desc.idProduct == USB_PRODUCT_ID)) {
			ibus = libusb_get_bus_number(devs[i]);
			if ((*bus != -1) && (*bus != ibus))
				continue;
			iport = libusb_get_port_number(devs[i]);
			if ((*port != -1) && (*port != iport))
				continue;

			ret = libusb_open(devs[i], &handle);
			if (ret == 0)
				break;

		}
	}
	libusb_free_device_list(devs, 0);
	if (!handle) {
		if (!suppres_nodev)
			fprintf(stderr, "device not found\n");
		return -1;
	}
	*bus = ibus;
	*port = iport;

//#ifdef _WIN32
		libusb_set_auto_detach_kernel_driver(handle, 1);
//#endif
	ret = libusb_claim_interface(handle, 0);
	if (ret != 0) {
		fprintf(stderr, "usb_claim_interface error: %s\n", libusb_error_name(ret));
		libusb_close(handle);
		handle = NULL;
		ret = -2;
	}

	return ret;
}

int NUC_OpenUsb(void)
{
	int b, p, ret;
	if (handle != NULL) return 0;

	b = usb_bus_cmdline;
	p = usb_port_cmdline;
	ret = NUC_OpenUsb_priv(&b, &p, 0);
	if (ret==0) {
		usb_bus_auto = b;
		usb_port_auto = p;
		printf("Device on bus %d : port %d\n",b, p);
	}
	return ret;
}

int NUC_OpenUsbRetry(int retrycount)
{
	int b, p, ret;
	if (handle != NULL) return 0;

	MSG_DEBUG("NUC_OpenUsbRetry: bus=%d port=%d\n", usb_bus_auto, usb_port_auto);

	b= usb_bus_auto;
	p= usb_port_auto;

	do {
		MSG_DEBUG("Connecting to xusb...%d\n", retrycount);
		ret = NUC_OpenUsb_priv(&b, &p, 1);
		if (ret != -1)
			break;
		sleep(1);
	}
	while (--retrycount);
	return ret;
}

void NUC_CloseUsb(void)
{
	if (handle)
	{
		libusb_release_interface(handle, 0);
		libusb_close(handle);
		handle=NULL;
	}
}

