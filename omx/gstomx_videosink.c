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

#include "gstomx_videosink.h"
#include "gstomx_base_sink.h"
#include "gstomx.h"

#include <string.h>             /* for strcmp */

GSTOMX_BOILERPLATE (GstOmxVideoSink, gst_omx_videosink, GstOmxBaseSink,
    GST_OMX_BASE_SINK_TYPE);

enum
{
  ARG_0,
  ARG_X_SCALE,
  ARG_Y_SCALE,
  ARG_ROTATION,
};

static void
type_base_init (gpointer g_class)
{
  GstElementClass *element_class;

  element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details_simple (element_class,
      "OpenMAX IL videosink element",
      "Video/Sink", "Renders video", "Felipe Contreras");

  gst_element_class_add_pad_template (element_class,
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
          gstomx_template_caps (G_TYPE_FROM_CLASS (g_class), "sink")));
}

static gboolean
setcaps (GstBaseSink * gst_sink, GstCaps * caps)
{
  GstOmxBaseSink *omx_base;
  GstOmxVideoSink *self;
  GOmxCore *gomx;

  omx_base = GST_OMX_BASE_SINK (gst_sink);
  self = GST_OMX_VIDEOSINK (gst_sink);
  gomx = (GOmxCore *) omx_base->gomx;

  GST_INFO_OBJECT (omx_base, "setcaps (sink): %" GST_PTR_FORMAT, caps);

  g_return_val_if_fail (gst_caps_get_size (caps) == 1, FALSE);

  {
    GstStructure *structure;
    const GValue *framerate = NULL;
    gint width;
    gint height;
    OMX_COLOR_FORMATTYPE color_format = OMX_COLOR_FormatUnused;

    structure = gst_caps_get_structure (caps, 0);

    gst_structure_get_int (structure, "width", &width);
    gst_structure_get_int (structure, "height", &height);

    if (strcmp (gst_structure_get_name (structure), "video/x-raw-yuv") == 0) {
      guint32 fourcc;

      framerate = gst_structure_get_value (structure, "framerate");

      if (gst_structure_get_fourcc (structure, "format", &fourcc)) {
        switch (fourcc) {
          case GST_MAKE_FOURCC ('I', '4', '2', '0'):
            color_format = OMX_COLOR_FormatYUV420PackedPlanar;
            break;
          case GST_MAKE_FOURCC ('Y', 'U', 'Y', '2'):
            color_format = OMX_COLOR_FormatYCbYCr;
            break;
          case GST_MAKE_FOURCC ('U', 'Y', 'V', 'Y'):
            color_format = OMX_COLOR_FormatCbYCrY;
            break;
        }
      }
    }

    {
      OMX_PARAM_PORTDEFINITIONTYPE param;

      G_OMX_INIT_PARAM (param);

      param.nPortIndex = omx_base->in_port->port_index;
      OMX_GetParameter (gomx->omx_handle, OMX_IndexParamPortDefinition, &param);

      switch (color_format) {
        case OMX_COLOR_FormatYUV420PackedPlanar:
          param.nBufferSize = (width * height * 1.5);
          break;
        case OMX_COLOR_FormatYCbYCr:
        case OMX_COLOR_FormatCbYCrY:
          param.nBufferSize = (width * height * 2);
          break;
        default:
          break;
      }

      param.format.video.nFrameWidth = width;
      param.format.video.nFrameHeight = height;
      param.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
      param.format.video.eColorFormat = color_format;
      if (framerate) {
        /* convert to Q.16 */
        param.format.video.xFramerate =
            (gst_value_get_fraction_numerator (framerate) << 16) /
            gst_value_get_fraction_denominator (framerate);
      }

      OMX_SetParameter (gomx->omx_handle, OMX_IndexParamPortDefinition, &param);
    }

    {
      OMX_CONFIG_ROTATIONTYPE config;

      G_OMX_INIT_PARAM (config);

      config.nPortIndex = omx_base->in_port->port_index;
      OMX_GetConfig (gomx->omx_handle, OMX_IndexConfigCommonScale, &config);

      config.nRotation = self->rotation;

      OMX_SetConfig (gomx->omx_handle, OMX_IndexConfigCommonRotate, &config);
    }

    {
      OMX_CONFIG_SCALEFACTORTYPE config;

      G_OMX_INIT_PARAM (config);

      config.nPortIndex = omx_base->in_port->port_index;
      OMX_GetConfig (gomx->omx_handle, OMX_IndexConfigCommonScale, &config);

      config.xWidth = self->x_scale;
      config.xHeight = self->y_scale;

      OMX_SetConfig (gomx->omx_handle, OMX_IndexConfigCommonScale, &config);
    }
  }

  return TRUE;
}

static void
set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstOmxVideoSink *self;

  self = GST_OMX_VIDEOSINK (object);

  switch (prop_id) {
    case ARG_X_SCALE:
      self->x_scale = g_value_get_uint (value);
      break;
    case ARG_Y_SCALE:
      self->y_scale = g_value_get_uint (value);
      break;
    case ARG_ROTATION:
      self->rotation = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstOmxVideoSink *self;

  self = GST_OMX_VIDEOSINK (object);

  switch (prop_id) {
    case ARG_X_SCALE:
      g_value_set_uint (value, self->x_scale);
      break;
    case ARG_Y_SCALE:
      g_value_set_uint (value, self->y_scale);
      break;
    case ARG_ROTATION:
      g_value_set_uint (value, self->rotation);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
type_class_init (gpointer g_class, gpointer class_data)
{
  GObjectClass *gobject_class;
  GstBaseSinkClass *gst_base_sink_class;

  gobject_class = (GObjectClass *) g_class;
  gst_base_sink_class = GST_BASE_SINK_CLASS (g_class);

  gst_base_sink_class->set_caps = setcaps;

  gobject_class->set_property = set_property;
  gobject_class->get_property = get_property;

  g_object_class_install_property (gobject_class, ARG_X_SCALE,
      g_param_spec_uint ("x-scale", "X Scale",
          "How much to scale the image in the X axis (100 means nothing)",
          0, G_MAXUINT, 100, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, ARG_Y_SCALE,
      g_param_spec_uint ("y-scale", "Y Scale",
          "How much to scale the image in the Y axis (100 means nothing)",
          0, G_MAXUINT, 100, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, ARG_ROTATION,
      g_param_spec_uint ("rotation", "Rotation",
          "Rotation angle",
          0, G_MAXUINT, 360, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void
type_instance_init (GTypeInstance * instance, gpointer g_class)
{
  GstOmxBaseSink *omx_base;

  omx_base = GST_OMX_BASE_SINK (instance);

  GST_DEBUG_OBJECT (omx_base, "start");
}
