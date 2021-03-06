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

#ifndef __IA_CSS_METADATA_H
#define __IA_CSS_METADATA_H

#include <type_support.h>
#include "ia_css_types.h"
#include "ia_css_stream_format.h"

/** Metadata configuration. This data structure contains necessary info
 *  to process sensor metadata.
 */
struct ia_css_metadata_config {
	enum ia_css_stream_format data_type; /**< Data type of CSI-2 embedded
			data. The default value is IA_CSS_STREAM_FORMAT_EMBEDDED. For
			certain sensors, user can choose non-default data type for embedded
			data. */
	struct ia_css_resolution  resolution; /**< Resolution */
};

struct ia_css_metadata_info {
	struct ia_css_resolution resolution; /**< Resolution */
	uint32_t                 stride;     /**< Stride in bytes */
	uint32_t                 size;       /**< Total size in bytes */
};

struct ia_css_metadata {
	struct ia_css_metadata_info info;    /**< Layout info */
	ia_css_ptr	            address; /**< CSS virtual address */
	uint32_t	            exp_id;  /**< Exposure ID */
};
#define SIZE_OF_IA_CSS_METADATA_STRUCT sizeof(struct ia_css_metadata)

/** @brief Allocate a metadata buffer.
 * @param[in]   metadata_info Metadata info struct, contains details on metadata buffers.
 * @return      Pointer of metadata buffer or NULL (if error)
 *
 * This function allocates a metadata buffer according to the properties
 * specified in the metadata_info struct.
 */
struct ia_css_metadata *
ia_css_metadata_allocate(const struct ia_css_metadata_info *metadata_info);

/** @brief Free a metadata buffer.
 *
 * @param[in]	metadata	Pointer of metadata buffer.
 * @return	None
 *
 * This function frees a metadata buffer.
 */
void
ia_css_metadata_free(struct ia_css_metadata *metadata);

#endif /* __IA_CSS_METADATA_H */
