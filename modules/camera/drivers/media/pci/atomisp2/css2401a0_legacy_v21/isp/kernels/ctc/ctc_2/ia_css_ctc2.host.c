/*
 * Support for Intel Camera Imaging ISP subsystem.
 *
 * Copyright (c) 2010 - 2014 Intel Corporation. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include "ia_css_types.h"
#include "sh_css_defs.h"
#include "ia_css_debug.h"
#include "assert_support.h"

#include "ctc/ctc_1.0/ia_css_ctc.host.h"
#include "ia_css_ctc2.host.h"

static void ctc_gradient(
	int *dydx, int *shift,
	int y1, int y0, int x1, int x0)
{
	int frc_bits = max(IA_CSS_CTC_COEF_SHIFT, 16);
	int dy = y1 - y0;
	int dx = x1 - x0;
	int dydx_int = dy / dx;
	int dydx_frc = ((dy - dydx_int * dx) << frc_bits) / dx;
	int sft;

	/* max_dydx = the maxinum gradient = the maximum y (gain) */
	int max_dydx = (1 << IA_CSS_CTC_COEF_SHIFT) - 1;

	assert(y0 >= 0 && y0 <= max_dydx);
	assert(y1 >= 0 && y1 <= max_dydx);
	assert(x0 < x1);
	assert(dydx != NULL);
	assert(shift != NULL);

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE_PRIVATE, "ctc_gradient() enter:\n");

	/* search "sft" which meets this condition:
		   (1 << (IA_CSS_CTC_COEF_SHIFT - 1))
		<= (((float)dy / (float)dx) * (1 << sft))
		<= ((1 << IA_CSS_CTC_COEF_SHIFT) - 1) */
	for (sft = 0; sft <= IA_CSS_CTC_COEF_SHIFT; sft++) {
		int tmp_dydx = (dydx_int << sft)
			     + (dydx_frc >> (frc_bits - sft));
		if (tmp_dydx <= max_dydx) {
			*dydx = tmp_dydx;
			*shift = sft;
		}
		if (tmp_dydx >= max_dydx)
			break;
	}

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE_PRIVATE, "ctc_gradient() leave:\n");
}

void
ia_css_ctc_encode(
	struct sh_css_isp_ctc_params *to,
	const struct ia_css_ctc_config *from)
{
	to->y0 = from->y0;
	to->y1 = from->y1;
	to->y2 = from->y2;
	to->y3 = from->y3;
	to->y4 = from->y4;
	to->y5 = from->y5;

	to->ce_gain_exp = from->ce_gain_exp;

	to->x1 = from->x1;
	to->x2 = from->x2;
	to->x3 = from->x3;
	to->x4 = from->x4;

	ctc_gradient(&(to->dydx0),
		     &(to->dydx0_shift),
		     from->y1, from->y0,
		     from->x1, 0);

	ctc_gradient(&(to->dydx1),
		     &(to->dydx1_shift),
		     from->y2, from->y1,
		     from->x2, from->x1);

	ctc_gradient(&to->dydx2,
		     &to->dydx2_shift,
		     from->y3, from->y2,
		     from->x3, from->x2);

	ctc_gradient(&to->dydx3,
		     &to->dydx3_shift,
		     from->y4, from->y3,
		     from->x4, from->x3);

	ctc_gradient(&(to->dydx4),
		     &(to->dydx4_shift),
		     from->y5, from->y4,
		     SH_CSS_BAYER_MAXVAL, from->x4);
}

void
ia_css_ctc_dump(
	const struct sh_css_isp_ctc_params *ctc,
	unsigned level);
