/*
 * Copyright (C) 2015 Red Hat
 * Copyright (C) 2017 Attila Szollosi
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/backlight.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>

#include <video/mipi_display.h>

static const char *regulator_names[] = {
	"vdd",
	"vdda",
        "vsp"
};

struct auo_panel {
	struct drm_panel base;
	struct mipi_dsi_device *dsi;

	struct backlight_device *backlight;
	struct regulator_bulk_data supplies[ARRAY_SIZE(regulator_names)];
	/*struct gpio_desc *enable_gpio;*/
	struct gpio_desc *te_gpio;
	struct gpio_desc *reset_gpio;

	bool prepared;
	bool enabled;

	const struct drm_display_mode *mode;
};

static inline struct auo_panel *to_auo_panel(struct drm_panel *panel)
{
	return container_of(panel, struct auo_panel, base);
}

static int auo_panel_cabc_early_init(struct auo_panel *auo)
{
	struct mipi_dsi_device *dsi = auo->dsi;
	int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0xee }, 1);
	if (ret < 0)
		return ret;
	printk("Got here\n");

	ret = mipi_dsi_dcs_write(dsi, 0x26, (u8[]){ 0x08 }, 1);
	if (ret < 0)
		return ret;
	msleep(1);

	ret = mipi_dsi_dcs_write(dsi, 0x26, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	return 0;
}

static int auo_panel_init(struct auo_panel *auo)
{
	struct mipi_dsi_device *dsi = auo->dsi;
	int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0xee }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xfb, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x12, (u8[]){ 0x50 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x13, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xfb, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0x4a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x01, (u8[]){ 0x43 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x02, (u8[]){ 0x54 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x03, (u8[]){ 0x55 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x04, (u8[]){ 0x55 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x05, (u8[]){ 0x33 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x06, (u8[]){ 0x22 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x08, (u8[]){ 0x56 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x09, (u8[]){ 0x8f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0b, (u8[]){ 0x97 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0c, (u8[]){ 0x97 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0d, (u8[]){ 0x2f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0e, (u8[]){ 0x24 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x36, (u8[]){ 0x73 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0f, (u8[]){ 0x04 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x69, (u8[]){ 0x99 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x6f, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0x05 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xfb, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x01, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x02, (u8[]){ 0x8d }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x03, (u8[]){ 0x8d }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x04, (u8[]){ 0x8d }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x05, (u8[]){ 0x30 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x06, (u8[]){ 0x33 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x07, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x08, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x09, (u8[]){ 0x46 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0a, (u8[]){ 0x46 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0d, (u8[]){ 0x0b }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0e, (u8[]){ 0x1d }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0f, (u8[]){ 0x08 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x10, (u8[]){ 0x53 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x11, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x12, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x14, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x15, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x16, (u8[]){ 0x05 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x17, (u8[]){ 0x04 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x19, (u8[]){ 0x7f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x1a, (u8[]){ 0xff }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x1b, (u8[]){ 0x0f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x1c, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x1d, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x1e, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x1f, (u8[]){ 0x07 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x20, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x21, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x22, (u8[]){ 0x55 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x23, (u8[]){ 0x4d }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x6c, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x6d, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x2d, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x83, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9e, (u8[]){ 0x58 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9f, (u8[]){ 0x58 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa0, (u8[]){ 0x41 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa2, (u8[]){ 0x10 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xbb, (u8[]){ 0x0a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xbc, (u8[]){ 0x0a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x28, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x2f, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x32, (u8[]){ 0x08 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x33, (u8[]){ 0xb8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x36, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x37, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x43, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x4b, (u8[]){ 0x21 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x4c, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x50, (u8[]){ 0x21 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x51, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x58, (u8[]){ 0x21 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x59, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x5d, (u8[]){ 0x21 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x5e, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xfb, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x75, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x76, (u8[]){ 0x59 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x77, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x78, (u8[]){ 0x61 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x79, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x7a, (u8[]){ 0x72 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x7b, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x7c, (u8[]){ 0x81 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x7d, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x7e, (u8[]){ 0x90 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x7f, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x80, (u8[]){ 0x9e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x81, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x82, (u8[]){ 0xab }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x83, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x84, (u8[]){ 0xb7 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x85, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x86, (u8[]){ 0xc2 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x87, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x88, (u8[]){ 0xe8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x89, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x8a, (u8[]){ 0x0a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x8b, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x8c, (u8[]){ 0x40 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x8d, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x8e, (u8[]){ 0x6e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x8f, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x90, (u8[]){ 0xb5 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x91, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x92, (u8[]){ 0x0d }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x93, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x94, (u8[]){ 0x0f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x95, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x96, (u8[]){ 0x57 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x97, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x98, (u8[]){ 0x9e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x99, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9a, (u8[]){ 0xca }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9b, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9c, (u8[]){ 0x05 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9d, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9e, (u8[]){ 0x2c }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9f, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa0, (u8[]){ 0x60 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa2, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa3, (u8[]){ 0x71 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa4, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa5, (u8[]){ 0x87 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa6, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa7, (u8[]){ 0xb8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa9, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xaa, (u8[]){ 0xc8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xab, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xac, (u8[]){ 0xd8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xad, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xae, (u8[]){ 0xe8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xaf, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb0, (u8[]){ 0xf8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb1, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb2, (u8[]){ 0xff }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb3, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb4, (u8[]){ 0x59 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb5, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb6, (u8[]){ 0x61 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb7, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb8, (u8[]){ 0x72 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb9, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xba, (u8[]){ 0x81 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xbb, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xbc, (u8[]){ 0x90 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xbd, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xbe, (u8[]){ 0x9e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xbf, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc0, (u8[]){ 0xab }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc1, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc2, (u8[]){ 0xb7 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc3, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc4, (u8[]){ 0xc2 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc5, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc6, (u8[]){ 0xe8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc7, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc8, (u8[]){ 0x0a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc9, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xca, (u8[]){ 0x40 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xcb, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xcc, (u8[]){ 0x6e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xcd, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xce, (u8[]){ 0xb5 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xcf, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd0, (u8[]){ 0x0d }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd1, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd2, (u8[]){ 0x0f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd3, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd4, (u8[]){ 0x57 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd5, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd6, (u8[]){ 0x9e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd7, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd8, (u8[]){ 0xca }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd9, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xda, (u8[]){ 0x05 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xdb, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xdc, (u8[]){ 0x2c }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xdd, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xde, (u8[]){ 0x60 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xdf, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe0, (u8[]){ 0x71 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe1, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe2, (u8[]){ 0x87 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe3, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe4, (u8[]){ 0xb8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe5, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe6, (u8[]){ 0xc8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe7, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe8, (u8[]){ 0xd8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe9, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xea, (u8[]){ 0xe8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xeb, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xec, (u8[]){ 0xf8 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xed, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xee, (u8[]){ 0xff }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xef, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xf0, (u8[]){ 0x6e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xf1, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xf2, (u8[]){ 0x76 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xf3, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xf4, (u8[]){ 0x85 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xf5, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xf6, (u8[]){ 0x92 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xf7, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xf8, (u8[]){ 0x9f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xf9, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xfa, (u8[]){ 0xab }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xfb, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x01, (u8[]){ 0xb6 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x02, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x03, (u8[]){ 0xc1 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x04, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x05, (u8[]){ 0xcb }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x06, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x07, (u8[]){ 0xef }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x08, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x09, (u8[]){ 0x0f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0a, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0b, (u8[]){ 0x43 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0c, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0d, (u8[]){ 0x6e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0e, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0f, (u8[]){ 0xb4 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x10, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x11, (u8[]){ 0x0c }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x12, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x13, (u8[]){ 0x0e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x14, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x15, (u8[]){ 0x56 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x16, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x17, (u8[]){ 0x9e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x18, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x19, (u8[]){ 0xca }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x1a, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x1b, (u8[]){ 0x05 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x1c, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x1d, (u8[]){ 0x2a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x1e, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x1f, (u8[]){ 0x56 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x20, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x21, (u8[]){ 0x61 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x22, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x23, (u8[]){ 0x6f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x24, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x25, (u8[]){ 0x7d }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x26, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x27, (u8[]){ 0x8d }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x28, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x29, (u8[]){ 0x96 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x2a, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x2b, (u8[]){ 0xa2 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x2d, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x2f, (u8[]){ 0xb4 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x30, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x31, (u8[]){ 0xff }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x32, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x33, (u8[]){ 0x6e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x34, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x35, (u8[]){ 0x76 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x36, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x37, (u8[]){ 0x85 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x38, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x39, (u8[]){ 0x92 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x3a, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x3b, (u8[]){ 0x9f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x3d, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x3f, (u8[]){ 0xab }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x40, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x41, (u8[]){ 0xb6 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x42, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x43, (u8[]){ 0xc1 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x44, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x45, (u8[]){ 0xcb }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x46, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x47, (u8[]){ 0xef }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x48, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x49, (u8[]){ 0x0f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x4a, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x4b, (u8[]){ 0x43 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x4c, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x4d, (u8[]){ 0x6e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x4e, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x4f, (u8[]){ 0xb4 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x50, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x51, (u8[]){ 0x0c }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x52, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x53, (u8[]){ 0x0e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x54, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x55, (u8[]){ 0x56 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x56, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x58, (u8[]){ 0x9e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x59, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x5a, (u8[]){ 0xca }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x5b, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x5c, (u8[]){ 0x05 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x5d, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x5e, (u8[]){ 0x2a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x5f, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x60, (u8[]){ 0x56 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x61, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x62, (u8[]){ 0x61 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x63, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x64, (u8[]){ 0x6f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x65, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x66, (u8[]){ 0x7d }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x67, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x68, (u8[]){ 0x8d }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x69, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x6a, (u8[]){ 0x96 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x6b, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x6c, (u8[]){ 0xa2 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x6d, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x6e, (u8[]){ 0xb4 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x6f, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x70, (u8[]){ 0xff }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x71, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x72, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x73, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x74, (u8[]){ 0x0e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x75, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x76, (u8[]){ 0x26 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x77, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x78, (u8[]){ 0x3a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x79, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x7a, (u8[]){ 0x4c }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x7b, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x7c, (u8[]){ 0x5e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x7d, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x7e, (u8[]){ 0x6e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x7f, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x80, (u8[]){ 0x7b }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x81, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x82, (u8[]){ 0x88 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x83, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x84, (u8[]){ 0xb4 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x85, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x86, (u8[]){ 0xdb }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x87, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x88, (u8[]){ 0x17 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x89, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x8a, (u8[]){ 0x4a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x8b, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x8c, (u8[]){ 0x98 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x8d, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x8e, (u8[]){ 0xfc }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x8f, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x90, (u8[]){ 0xfe }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x91, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x92, (u8[]){ 0x4a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x93, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x94, (u8[]){ 0x93 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x95, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x96, (u8[]){ 0xc0 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x97, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x98, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x99, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9a, (u8[]){ 0x2f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9b, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9c, (u8[]){ 0x85 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9d, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9e, (u8[]){ 0xcc }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x9f, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa0, (u8[]){ 0xd4 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa2, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa3, (u8[]){ 0xdc }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa4, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa5, (u8[]){ 0xe4 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa6, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa7, (u8[]){ 0xec }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xa9, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xaa, (u8[]){ 0xf4 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xab, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xac, (u8[]){ 0xfc }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xad, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xae, (u8[]){ 0xff }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xaf, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb0, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb1, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb2, (u8[]){ 0x0e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb3, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb4, (u8[]){ 0x26 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb5, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb6, (u8[]){ 0x3a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb7, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb8, (u8[]){ 0x4c }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xb9, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xba, (u8[]){ 0x5e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xbb, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xbc, (u8[]){ 0x6e }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xbd, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xbe, (u8[]){ 0x7b }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xbf, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc0, (u8[]){ 0x88 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc1, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc2, (u8[]){ 0xb4 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc3, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc4, (u8[]){ 0xdb }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc5, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc6, (u8[]){ 0x17 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc7, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc8, (u8[]){ 0x4a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc9, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xca, (u8[]){ 0x98 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xcb, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xcc, (u8[]){ 0xfc }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xcd, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xce, (u8[]){ 0xfe }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xcf, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd0, (u8[]){ 0x4a }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd1, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd2, (u8[]){ 0x93 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd3, (u8[]){ 0x02 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd4, (u8[]){ 0xc0 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd5, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd6, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd7, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd8, (u8[]){ 0x2f }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xd9, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xda, (u8[]){ 0x85 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xdb, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xdc, (u8[]){ 0xcc }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xdd, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xde, (u8[]){ 0xd4 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xdf, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe0, (u8[]){ 0xdc }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe1, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe2, (u8[]){ 0xe4 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe3, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe4, (u8[]){ 0xec }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe5, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe6, (u8[]){ 0xf4 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe7, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe8, (u8[]){ 0xfc }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xe9, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xea, (u8[]){ 0xff }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xfb, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xc2, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x3b, (u8[]){ 0x03, 0x02, 0x03, 0x08, 0x04 }, 5);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xba, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0)
		return ret;

	msleep(50);

	return 0;
}

static int auo_panel_cabc_early_on(struct auo_panel *auo)
{
	struct mipi_dsi_device *dsi = auo->dsi;
	int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0x04 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x2a, (u8[]){ 0x84 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x29, (u8[]){ 0x99 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x28, (u8[]){ 0x99 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x27, (u8[]){ 0x99 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x26, (u8[]){ 0xaa }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x25, (u8[]){ 0xbb }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x24, (u8[]){ 0xcc }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x23, (u8[]){ 0xdd }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x22, (u8[]){ 0xee }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x21, (u8[]){ 0xff }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x0a, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x32, (u8[]){ 0x64 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xfb, (u8[]){ 0x01 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0x00 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x51, (u8[]){ 0xff }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x53, (u8[]){ 0x2c }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_write(dsi, 0x55, (u8[]){ 0x03 }, 1);
	if (ret < 0)
		return ret;

	return 0;
}

static int auo_panel_on(struct auo_panel *auo)
{
	struct mipi_dsi_device *dsi = auo->dsi;
	int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0)
		return ret;

	msleep(40);

	return 0;
}

static int auo_panel_off(struct auo_panel *auo)
{
	struct mipi_dsi_device *dsi = auo->dsi;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0x10 }, 1);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_set_display_off(dsi);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
	if (ret < 0)
		return ret;

	msleep(100);

	return 0;
}

static int auo_panel_disable(struct drm_panel *panel)
{
	struct auo_panel *auo = to_auo_panel(panel);

	if (!auo->enabled)
		return 0;

	DRM_DEBUG("disable\n");

	if (auo->backlight) {
		auo->backlight->props.power = FB_BLANK_POWERDOWN;
		backlight_update_status(auo->backlight);
	}

	auo->enabled = false;

	return 0;
}

static int auo_panel_unprepare(struct drm_panel *panel)
{
	struct auo_panel *auo = to_auo_panel(panel);
	int ret;

	if (!auo->prepared)
		return 0;

	DRM_DEBUG("unprepare\n");

	ret = auo_panel_off(auo);
	if (ret) {
		dev_err(panel->dev, "failed to set panel off: %d\n", ret);
		return ret;
	}

	/*regulator_disable(auo->supply);*/
	ret = regulator_bulk_disable(ARRAY_SIZE(auo->supplies),
				      auo->supplies);
	if (ret < 0) {
		dev_err(panel->dev, "failed to enable regulators, ret=%d\n", ret);
		return ret;
	}

	/*if (auo->enable_gpio)*/
		/*gpiod_set_value(auo->enable_gpio, 0);*/

	if (auo->reset_gpio)
		gpiod_set_value(auo->reset_gpio, 0);

	auo->prepared = false;

	return 0;
}

static int auo_panel_prepare(struct drm_panel *panel)
{
	struct auo_panel *auo = to_auo_panel(panel);
	int ret;

	if (auo->prepared)
		return 0;

	DRM_DEBUG("prepare\n");

	/*if (auo->reset_gpio) {*/
		/*gpiod_set_value(auo->reset_gpio, 0);*/
		/*msleep(5);*/
	/*}*/

	/*ret = regulator_enable(auo->supply);*/
	/*if (ret < 0)*/
		/*return ret;*/

        msleep(200);
	ret = regulator_bulk_enable(ARRAY_SIZE(auo->supplies),
				      auo->supplies);
	if (ret < 0) {
		dev_err(panel->dev, "failed to enable regulators, ret=%d\n", ret);
		return ret;
	} else {
                printk("regulators successfully enabled!!!");
        }

	msleep(10);

	/*if (auo->enable_gpio) {*/
	    /*gpiod_set_value(auo->enable_gpio, 1);*/
	    /*usleep_range(10, 20);*/
	/*}*/

	/*if (auo->te_gpio) {*/
	    /*gpiod_set_value(auo->te_gpio, 1);*/
	    /*usleep_range(10, 20);*/
	/*}*/

	if (auo->reset_gpio) {
		gpiod_set_value(auo->reset_gpio, 1);
		msleep(20);
		gpiod_set_value(auo->reset_gpio, 0);
		msleep(200);
		gpiod_set_value(auo->reset_gpio, 1);
	}

	msleep(20);

	ret = auo_panel_cabc_early_init(auo);
	if (ret) {
		dev_err(panel->dev, "early init sequence failed: %d\n", ret);
		goto poweroff;
	}

	ret = auo_panel_cabc_early_on(auo);
	if (ret) {
		dev_err(panel->dev, "early on sequence failed: %d\n", ret);
		goto poweroff;
	}

	ret = auo_panel_init(auo);
	if (ret) {
		dev_err(panel->dev, "failed to init panel: %d\n", ret);
		goto poweroff;
	}

	ret = auo_panel_on(auo);
	if (ret) {
		dev_err(panel->dev, "failed to set panel on: %d\n", ret);
		goto poweroff;
	}

	auo->prepared = true;

	return 0;

poweroff:
	/*regulator_disable(auo->supply);*/
	ret = regulator_bulk_disable(ARRAY_SIZE(auo->supplies),
				      auo->supplies);
	if (ret < 0) {
		dev_err(panel->dev, "failed to enable regulators, ret=%d\n", ret);
	}

	/*if (auo->enable_gpio)*/
		/*gpiod_set_value(auo->enable_gpio, 0);*/

	if (auo->reset_gpio)
		gpiod_set_value(auo->reset_gpio, 0);
	return ret;
}

static int auo_panel_enable(struct drm_panel *panel)
{
	struct auo_panel *auo = to_auo_panel(panel);

	if (auo->enabled)
		return 0;

	DRM_DEBUG("enable\n");

	if (auo->backlight) {
		auo->backlight->props.power = FB_BLANK_UNBLANK;
		backlight_update_status(auo->backlight);
	}

	auo->enabled = true;

	return 0;
}

static const struct drm_display_mode default_mode = {
		.clock = 71331,
		.hdisplay = 720,
		.hsync_start = 720 + 136,
		.hsync_end = 720 + 136 + 4,
		.htotal = 720 + 136 + 4 + 4,
		.vdisplay = 1280,
		.vsync_start = 1280 + 94,
		.vsync_end = 1280 + 94 + 1,
		.vtotal = 1280 + 94 + 1 + 1,
		.vrefresh = 60,
};

static int auo_panel_get_modes(struct drm_panel *panel)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(panel->drm, &default_mode);
	if (!mode) {
		dev_err(panel->drm->dev, "failed to add mode %ux%ux@%u\n",
				default_mode.hdisplay, default_mode.vdisplay,
				default_mode.vrefresh);
		return -ENOMEM;
	}

	drm_mode_set_name(mode);

	drm_mode_probed_add(panel->connector, mode);

	panel->connector->display_info.width_mm = 53;
	panel->connector->display_info.height_mm = 94;

	return 1;
}

static const struct drm_panel_funcs auo_panel_funcs = {
		.disable = auo_panel_disable,
		.unprepare = auo_panel_unprepare,
		.prepare = auo_panel_prepare,
		.enable = auo_panel_enable,
		.get_modes = auo_panel_get_modes,
};

static const struct of_device_id auo_of_match[] = {
		{ .compatible = "auo,novatek-720p-vid", },
		{ }
};
MODULE_DEVICE_TABLE(of, auo_of_match);

static int auo_panel_add(struct auo_panel *auo)
{
	struct device *dev= &auo->dsi->dev;
	struct device_node *np;
	int ret;
	unsigned int i;

	auo->mode = &default_mode;

	for (i = 0; i < ARRAY_SIZE(auo->supplies); i++)
		auo->supplies[i].supply = regulator_names[i];

	ret = devm_regulator_bulk_get(dev, ARRAY_SIZE(auo->supplies),
				      auo->supplies);
	if (ret < 0) {
		dev_err(dev, "failed to init regulator, ret=%d\n", ret);
		return ret;
	}

	/*auo->supply = devm_regulator_get(dev, "power");*/
	/*if (IS_ERR(auo->supply))*/
		/*return PTR_ERR(auo->supply);*/

	/*auo->enable_gpio = devm_gpiod_get(dev, "enable", GPIOD_OUT_LOW);*/
	/*if (IS_ERR(auo->enable_gpio)) {*/
		/*ret = PTR_ERR(auo->enable_gpio);*/
		/*dev_err(dev, "cannot get enable-gpio %d\n", ret);*/
		/*return ret;*/
	/*}*/

	/*auo->te_gpio = devm_gpiod_get(dev, "te", GPIOD_OUT_LOW);*/
	/*if (IS_ERR(auo->te_gpio)) {*/
		/*ret = PTR_ERR(auo->te_gpio);*/
		/*dev_err(dev, "cannot get te-gpio %d\n", ret);*/
		/*return ret;*/
	/*}*/

	/*auo->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);*/
	/*if (IS_ERR(auo->reset_gpio)) {*/
		/*dev_err(dev, "cannot get reset-gpios %ld\n",*/
			/*PTR_ERR(auo->reset_gpio));*/
		/*auo->reset_gpio = NULL;*/
	/*} else {*/
		/*gpiod_direction_output(auo->reset_gpio, 0);*/
	/*}*/

	np = of_parse_phandle(dev->of_node, "backlight", 0);
	if (np) {
		auo->backlight = of_find_backlight_by_node(np);
		of_node_put(np);

		if (!auo->backlight)
			return -EPROBE_DEFER;
	}

	drm_panel_init(&auo->base);
	auo->base.funcs = &auo_panel_funcs;
	auo->base.dev = &auo->dsi->dev;

	ret = drm_panel_add(&auo->base);
	if (ret < 0)
		goto put_backlight;

	return 0;

	put_backlight:
	if (auo->backlight)
		put_device(&auo->backlight->dev);

	return ret;
}

static void auo_panel_del(struct auo_panel *auo)
{
	if (auo->base.dev)
		drm_panel_remove(&auo->base);

	if (auo->backlight)
		put_device(&auo->backlight->dev);
}

static int auo_panel_probe(struct mipi_dsi_device *dsi)
{
	struct auo_panel *auo;
	int ret;

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO |
			MIPI_DSI_MODE_VIDEO_HSE |
			MIPI_DSI_CLOCK_NON_CONTINUOUS |
			MIPI_DSI_MODE_EOT_PACKET;

	auo = devm_kzalloc(&dsi->dev, sizeof(*auo), GFP_KERNEL);
	if (!auo) {
		return -ENOMEM;
	}

	mipi_dsi_set_drvdata(dsi, auo);

	auo->dsi = dsi;

	ret = auo_panel_add(auo);
	if (ret < 0) {
		return ret;
	}

	return mipi_dsi_attach(dsi);
}

static int auo_panel_remove(struct mipi_dsi_device *dsi)
{
	struct auo_panel *auo = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = auo_panel_disable(&auo->base);
	if (ret < 0)
		dev_err(&dsi->dev, "failed to disable panel: %d\n", ret);

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "failed to detach from DSI host: %d\n", ret);

	drm_panel_detach(&auo->base);
	auo_panel_del(auo);

	return 0;
}

static void auo_panel_shutdown(struct mipi_dsi_device *dsi)
{
	struct auo_panel *auo = mipi_dsi_get_drvdata(dsi);

	auo_panel_disable(&auo->base);
}

static struct mipi_dsi_driver auo_panel_driver = {
	.driver = {
		.name = "panel-auo-novatek-720p",
		.of_match_table = auo_of_match,
	},
	.probe = auo_panel_probe,
	.remove = auo_panel_remove,
	.shutdown = auo_panel_shutdown,
};
module_mipi_dsi_driver(auo_panel_driver);

MODULE_AUTHOR("Attila Szollosi <ata2001@airmail.cc>");
MODULE_DESCRIPTION("AUO Novatek 720p panel driver");
MODULE_LICENSE("GPL v2");
