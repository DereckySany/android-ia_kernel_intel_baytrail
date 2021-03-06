/* Special Initializers for certain USB Mass Storage devices
 *
 * Current development and maintenance by:
 *   (c) 1999, 2000 Matthew Dharm (mdharm-usb@one-eyed-alien.net)
 *
 * This driver is based on the 'USB Mass Storage Class' document. This
 * describes in detail the protocol used to communicate with such
 * devices.  Clearly, the designers had SCSI and ATAPI commands in
 * mind when they created this document.  The commands are all very
 * similar to commands in the SCSI-II and ATAPI specifications.
 *
 * It is important to note that in a number of cases this class
 * exhibits class-specific exemptions from the USB specification.
 * Notably the usage of NAK, STALL and ACK differs from the norm, in
 * that they are used to communicate wait, failed and OK on commands.
 *
 * Also, for certain devices, the interrupt endpoint is used to convey
 * status of a command.
 *
 * Please see http://www.one-eyed-alien.net/~mdharm/linux-usb for more
 * information about this driver.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/errno.h>

#include "usb.h"
#include "initializers.h"
#include "debug.h"
#include "transport.h"

/* This places the Shuttle/SCM USB<->SCSI bridge devices in multi-target
 * mode */
int usb_stor_euscsi_init(struct us_data *us)
{
	int result;

	usb_stor_dbg(us, "Attempting to init eUSCSI bridge...\n");
	us->iobuf[0] = 0x1;
	result = usb_stor_control_msg(us, us->send_ctrl_pipe,
			0x0C, USB_RECIP_INTERFACE | USB_TYPE_VENDOR,
			0x01, 0x0, us->iobuf, 0x1, 5000);
	usb_stor_dbg(us, "-- result is %d\n", result);

	return 0;
}

/* This function is required to activate all four slots on the UCR-61S2B
 * flash reader */
int usb_stor_ucr61s2b_init(struct us_data *us)
{
	struct bulk_cb_wrap *bcb = (struct bulk_cb_wrap*) us->iobuf;
	struct bulk_cs_wrap *bcs = (struct bulk_cs_wrap*) us->iobuf;
	int res;
	unsigned int partial;
	static char init_string[] = "\xec\x0a\x06\x00$PCCHIPS";

	usb_stor_dbg(us, "Sending UCR-61S2B initialization packet...\n");

	bcb->Signature = cpu_to_le32(US_BULK_CB_SIGN);
	bcb->Tag = 0;
	bcb->DataTransferLength = cpu_to_le32(0);
	bcb->Flags = bcb->Lun = 0;
	bcb->Length = sizeof(init_string) - 1;
	memset(bcb->CDB, 0, sizeof(bcb->CDB));
	memcpy(bcb->CDB, init_string, sizeof(init_string) - 1);

	res = usb_stor_bulk_transfer_buf(us, us->send_bulk_pipe, bcb,
			US_BULK_CB_WRAP_LEN, &partial);
	if (res)
		return -EIO;

	usb_stor_dbg(us, "Getting status packet...\n");
	res = usb_stor_bulk_transfer_buf(us, us->recv_bulk_pipe, bcs,
			US_BULK_CS_WRAP_LEN, &partial);
	if (res)
		return -EIO;

	return 0;
}

/* This places the HUAWEI E220 devices in multi-port mode */
int usb_stor_huawei_e220_init(struct us_data *us)
{
	int result;

	result = usb_stor_control_msg(us, us->send_ctrl_pipe,
				      USB_REQ_SET_FEATURE,
				      USB_TYPE_STANDARD | USB_RECIP_DEVICE,
				      0x01, 0x0, NULL, 0x0, 1000);
	usb_stor_dbg(us, "Huawei mode set result is %d\n", result);
	return 0;
}

/*hw dongle +*/
#define IS_HUAWEI_DONGLES 1
#define NOT_HUAWEI_DONGLES 0

static int usb_stor_huawei_dongles_pid(struct us_data *us)
{
	int ret = NOT_HUAWEI_DONGLES;
	struct usb_interface_descriptor *idesc = NULL;
	
	idesc = &us->pusb_intf->cur_altsetting->desc;
	
	if (NULL != idesc ) {
		if ( (0x0000 == idesc->bInterfaceNumber)) {
			if ((0x1401 <= us->pusb_dev->descriptor.idProduct && 0x1600 >=
				us->pusb_dev->descriptor.idProduct)
				|| (0x1c02 <= us->pusb_dev->descriptor.idProduct &&
				0x2202 >= us->pusb_dev->descriptor.idProduct)
				|| (0x1001 == us->pusb_dev->descriptor.idProduct)
				|| (0x1003 == us->pusb_dev->descriptor.idProduct)
				|| (0x1004 == us->pusb_dev->descriptor.idProduct)) {
					if ((0x1501 <= us->pusb_dev->descriptor.idProduct) && (0x1504 >=
						us->pusb_dev->descriptor.idProduct)) {
						ret = NOT_HUAWEI_DONGLES;
					} else {
						ret = IS_HUAWEI_DONGLES;
					}
			}
		}
	}
	
	return ret;
	
}

/*
 * It will send a scsi switch command called rewind' to huawei dongle.
 * When the dongle receives this command at the first time,
 * it will reboot immediately. After rebooted, it will ignore this command.
 * So it is  unnecessary to read its response.
 */
static int usb_stor_huawei_scsi_init_1446(struct us_data *us)
{
        int result = 0;
        int act_len = 0;
        struct bulk_cb_wrap *bcbw = (struct bulk_cb_wrap *) us->iobuf;
        char rewind_cmd[] = {0x11, 0x06, 0x20, 0x00, 0x00, 0x01, 0x01, 0x00,
							 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        bcbw->Signature = cpu_to_le32(US_BULK_CB_SIGN);
        bcbw->Tag = 0;
        bcbw->DataTransferLength = 0;
        bcbw->Flags = bcbw->Lun = 0;
        bcbw->Length = sizeof(rewind_cmd);
        memset(bcbw->CDB, 0, sizeof(bcbw->CDB));
        memcpy(bcbw->CDB, rewind_cmd, sizeof(rewind_cmd));

        result = usb_stor_bulk_transfer_buf(us, us->send_bulk_pipe, bcbw,
											US_BULK_CB_WRAP_LEN, &act_len);
        usb_stor_dbg(us, "transfer actual length=%d, result=%d\n", act_len, result);
        return result;
}

int usb_stor_huawei_scsi_init(struct us_data *us)
{
	int result = 0;
	int act_len = 0;
	unsigned char cmd[32] = {0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11,
	0x06, 0x30, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
	result = usb_stor_bulk_transfer_buf (us, us->send_bulk_pipe, cmd, 31, &act_len);
	
	usb_stor_dbg(us, "usb_stor_bulk_transfer_buf performing result is %d, transfer the actual length=%d\n", result, act_len);
	
	return result;
}

int usb_stor_huawei_init(struct us_data *us)
{
	int result = 0;
	
	if(usb_stor_huawei_dongles_pid(us)){
		if ((0x1446 < le16_to_cpu(us->pusb_dev->descriptor.idProduct))){
			result = usb_stor_huawei_scsi_init(us);
		} else if ((0x1446 == le16_to_cpu(us->pusb_dev->descriptor.idProduct))){
			result = usb_stor_huawei_scsi_init_1446(us);
		} else {
			result = usb_stor_huawei_e220_init(us);
		}
	}
	return result;
}

/*hw dongle -*/

/* This functinal is used for ZTE USB devices. added by Mario Lu from intel */
int usb_stor_zte_init(struct us_data *us)
{
	int result;

	result = usb_stor_control_msg(us, us->send_ctrl_pipe,
				      0xB0,/*request*/
				      0xC0,/*requesttype*/
				      0x0007/*value*/, 0x0, NULL, 0x0, 1000);
	usb_stor_dbg(us, "usb_stor_zte_init result is %d\n", result);
	return 0;
}
int usb_stor_ZTE_AC580_init(struct us_data *us) // PID = 0x0026
{
	int result;
	result = usb_stor_control_msg(us, us->send_ctrl_pipe,
				      0xA1, /* request */
				      0xC0, /* requesttype*/
				      0x01, 0x0, NULL, 0x0, 1000);
	usb_stor_dbg(us,"zte usb_stor_zte_init result is %d\n", result);
	return 0;
}
