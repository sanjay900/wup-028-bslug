#include "button_map.h"
#include "usb_hid.h"
#include "wiimote.h"
#include "rvl/WPAD.h"
#include <stdio.h>


static inline int santroller_request_data(usb_input_device_t *device)
{
	return 0;
}


bool santroller_driver_ops_probe(uint16_t vid, uint16_t pid)
{
	static const struct device_id_t compatible[] = {
		{SANTROLLER_VID, SANTROLLER_PID},
	};

	return usb_driver_is_comaptible(vid, pid, compatible, ARRAY_SIZE(compatible));
}

int santroller_driver_ops_init(usb_input_device_t *device)
{
	int ret;
	if (device->extensionCallback) {
		device->extensionCallback(device->wiimote, WPAD_EXTENSION_GUITAR);
	}
	device->extension = WPAD_EXTENSION_GUITAR;
	device->wpadData.extension = WPAD_EXTENSION_GUITAR;
	device->format = WPAD_FORMAT_GUITAR;
	// Send the ctrl transfer fakemote sends, to jump to PS3 mode. Not usually necessary but helps for dolphin
	ret = usb_device_driver_issue_ctrl_transfer_async(device, 0xa1, 0x01, 0x03f2, 0x00, device->usb_async_resp,
							   0x11);
	if (ret < 0)
		return ret;

	return 0;
}

static int santroller_driver_update_leds(usb_input_device_t *device)
{
	// TODO: this
	return 0;
}

int santroller_driver_ops_disconnect(usb_input_device_t *device)
{
	struct santroller_private_data_t *priv = (void *)device->private_data;

	return santroller_driver_update_leds(device);
}

int santroller_driver_ops_slot_changed(usb_input_device_t *device, uint8_t slot)
{
	struct santroller_private_data_t *priv = (void *)device->private_data;

	return santroller_driver_update_leds(device);
}

bool santroller_report_input(usb_input_device_t *device)
{
	return true;
}
static uint32_t cpu_isr_disable(void) {
    uint32_t isr, tmp;
    asm volatile("mfmsr %0; rlwinm %1, %0, 0, 0xFFFF7FFF; mtmsr %1" : "=r"(isr), "=r"(tmp));
    return isr;
}
static void cpu_isr_restore(uint32_t isr) {
    uint32_t tmp;
    asm volatile("mfmsr %0; rlwimi %0, %1, 0, 0x8000; mtmsr %0" : "=&r"(tmp) : "r"(isr));
}
int santroller_driver_ops_usb_async_resp(usb_input_device_t *device)
{
	uint32_t isr = cpu_isr_disable();
	cpu_isr_restore(isr);
	return santroller_request_data(device);
}

const usb_device_driver_t santroller_usb_device_driver = {
	.probe		= santroller_driver_ops_probe,
	.init		= santroller_driver_ops_init,
	.disconnect	= santroller_driver_ops_disconnect,
	.slot_changed	= santroller_driver_ops_slot_changed,
	.report_input	= santroller_report_input,
	.usb_async_resp	= santroller_driver_ops_usb_async_resp,
};
