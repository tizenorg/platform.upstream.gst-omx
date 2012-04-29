/*
 * Copyright (C) 2007-2009 Nokia Corporation.
 *
 * Author: Felipe Contreras <felipe.contreras@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef GSTOMX_BASE_FILTER_H
#define GSTOMX_BASE_FILTER_H

#include <gst/gst.h>
#include <gst/base/gstadapter.h>
#include "gstomx_util.h"
#include <async_queue.h>

G_BEGIN_DECLS
#define GST_OMX_BASE_FILTER_TYPE (gst_omx_base_filter_get_type ())
#define GST_OMX_BASE_FILTER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_OMX_BASE_FILTER_TYPE, GstOmxBaseFilter))
#define GST_OMX_BASE_FILTER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_OMX_BASE_FILTER_TYPE, GstOmxBaseFilterClass))
#define GST_OMX_BASE_FILTER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_OMX_BASE_FILTER_TYPE, GstOmxBaseFilterClass))
typedef struct GstOmxBaseFilter GstOmxBaseFilter;
typedef struct GstOmxBaseFilterClass GstOmxBaseFilterClass;
typedef void (*GstOmxBaseFilterCb) (GstOmxBaseFilter * self);
typedef gboolean (*GstOmxBaseFilterEventCb) (GstPad * pad, GstEvent * event);


/* Add extended_color_format */
typedef enum _EXT_OMX_COLOR_FORMATTYPE {
    OMX_EXT_COLOR_FormatNV12TPhysicalAddress = 0x7F000001, /**< Reserved region for introducing Vendor Extensions */
    OMX_EXT_COLOR_FormatNV12LPhysicalAddress = 0x7F000002,
    OMX_EXT_COLOR_FormatNV12Tiled            = 0x7FC00002  /* 0x7FC00002 */
}EXT_OMX_COLOR_FORMATTYPE;

typedef enum GstOmxChangeState
{
    GstOmx_ToLoaded,
    GstOmx_LodedToIdle,
    GstOmx_IdleToExcuting
}GstOmxChangeState;

struct GstOmxBaseFilter
{
  GstElement element;

  GstPad *sinkpad;
  GstPad *srcpad;

  GOmxCore *gomx;
  GOmxPort *in_port;
  GOmxPort *out_port;

  gboolean use_timestamps;   /** @todo remove; timestamps should always be used */
  gboolean ready;
  GMutex *ready_lock;

  GstOmxBaseFilterCb omx_setup;
  GstOmxBaseFilterEventCb omx_event;
  GstFlowReturn last_pad_push_return;
  GstBuffer *codec_data;

    /** @todo these are hacks, OpenMAX IL spec should be revised. */
  gboolean share_input_buffer;
  gboolean share_output_buffer;


  gboolean use_state_tuning;

  GstAdapter *adapter;  /* adapter */
  guint adapter_size;
};

struct GstOmxBaseFilterClass
{
  GstElementClass parent_class;

  void (*process_input_buf)(GstOmxBaseFilter * omx_base_filter, GstBuffer * buf);
  void (*process_output_buf)(GstOmxBaseFilter * omx_base_filter, GstBuffer * buf, OMX_BUFFERHEADERTYPE *omx_buffer);
};

GType gst_omx_base_filter_get_type (void);

G_END_DECLS
#endif /* GSTOMX_BASE_FILTER_H */
