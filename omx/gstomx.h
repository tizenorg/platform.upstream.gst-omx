/*
 * Copyright (C) 2011, Hewlett-Packard Development Company, L.P.
 *   Author: Sebastian Dröge <sebastian.droege@collabora.co.uk>, Collabora Ltd.
 * Copyright (C) 2013, Collabora Ltd.
 *   Author: Sebastian Dröge <sebastian.droege@collabora.co.uk>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef __GST_OMX_H__
#define __GST_OMX_H__

#include <gmodule.h>
#include <gst/gst.h>
#include <string.h>
#include <mm_types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef GST_OMX_STRUCT_PACKING
# if GST_OMX_STRUCT_PACKING == 1
#  pragma pack(1)
# elif GST_OMX_STRUCT_PACKING == 2
#  pragma pack(2)
# elif GST_OMX_STRUCT_PACKING == 4
#  pragma pack(4)
# elif GST_OMX_STRUCT_PACKING == 8
#  pragma pack(8)
# else
#  error "Unsupported struct packing value"
# endif
#endif

#include <OMX_Core.h>
#include <OMX_Component.h>

#include <tbm_type.h>
#include <tbm_surface.h>
#include <tbm_bufmgr.h>
#ifdef GST_OMX_STRUCT_PACKING
#pragma pack()
#endif

G_BEGIN_DECLS

#define GST_OMX_INIT_STRUCT(st) G_STMT_START { \
  memset ((st), 0, sizeof (*(st))); \
  (st)->nSize = sizeof (*(st)); \
  (st)->nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
  (st)->nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
  (st)->nVersion.s.nRevision = OMX_VERSION_REVISION; \
  (st)->nVersion.s.nStep = OMX_VERSION_STEP; \
} G_STMT_END

/* Different hacks that are required to work around
 * bugs in different OpenMAX implementations
 */
/* In the EventSettingsChanged callback use nData2 instead of nData1 for
 * the port index. Happens with Bellagio.
 */
#define GST_OMX_HACK_EVENT_PORT_SETTINGS_CHANGED_NDATA_PARAMETER_SWAP G_GUINT64_CONSTANT (0x0000000000000001)
/* In the EventSettingsChanged callback assume that port index 0 really
 * means port index 1. Happens with the Bellagio ffmpegdist video decoder.
 */
#define GST_OMX_HACK_EVENT_PORT_SETTINGS_CHANGED_PORT_0_TO_1          G_GUINT64_CONSTANT (0x0000000000000002)
/* If the video framerate is not specified as fraction (Q.16) but as
 * integer number. Happens with the Bellagio ffmpegdist video encoder.
 */
#define GST_OMX_HACK_VIDEO_FRAMERATE_INTEGER                          G_GUINT64_CONSTANT (0x0000000000000004)
/* If the SYNCFRAME flag on encoder output buffers is not used and we
 * have to assume that all frames are sync frames.
 * Happens with the Bellagio ffmpegdist video encoder.
 */
#define GST_OMX_HACK_SYNCFRAME_FLAG_NOT_USED                          G_GUINT64_CONSTANT (0x0000000000000008)
/* If the component needs to be re-created if the caps change.
 * Happens with Qualcomm's OpenMAX implementation.
 */
#define GST_OMX_HACK_NO_COMPONENT_RECONFIGURE                         G_GUINT64_CONSTANT (0x0000000000000010)

/* If the component does not accept empty EOS buffers.
 * Happens with Qualcomm's OpenMAX implementation.
 */
#define GST_OMX_HACK_NO_EMPTY_EOS_BUFFER                              G_GUINT64_CONSTANT (0x0000000000000020)

/* If the component might not acknowledge a drain.
 * Happens with TI's Ducati OpenMAX implementation.
 */
#define GST_OMX_HACK_DRAIN_MAY_NOT_RETURN                             G_GUINT64_CONSTANT (0x0000000000000040)

/* If the component doesn't allow any component role to be set.
 * Happens with Broadcom's OpenMAX implementation.
 */
#define GST_OMX_HACK_NO_COMPONENT_ROLE                                G_GUINT64_CONSTANT (0x0000000000000080)

typedef struct _GstOMXCore GstOMXCore;
typedef struct _GstOMXPort GstOMXPort;
typedef enum _GstOMXPortDirection GstOMXPortDirection;
typedef struct _GstOMXComponent GstOMXComponent;
typedef struct _GstOMXBuffer GstOMXBuffer;
typedef struct _GstOMXClassData GstOMXClassData;
typedef struct _GstOMXMessage GstOMXMessage;

/* MODIFICATION */
typedef enum GOmxVendor GOmxVendor; /* check omx vender */

/* using common scmn_imgb format */
#define SCMN_IMGB_MAX_PLANE         (4) /* max channel count */

/* image buffer definition
    +------------------------------------------+ ---
    |                                          |  ^
    |     a[], p[]                             |  |
    |     +---------------------------+ ---    |  |
    |     |                           |  ^     |  |
    |     |<---------- w[] ---------->|  |     |  |
    |     |                           |  |     |  |
    |     |                           |        |
    |     |                           |  h[]   |  e[]
    |     |                           |        |
    |     |                           |  |     |  |
    |     |                           |  |     |  |
    |     |                           |  v     |  |
    |     +---------------------------+ ---    |  |
    |                                          |  v
    +------------------------------------------+ ---

    |<----------------- s[] ------------------>|
*/

typedef struct
{
    int      w[SCMN_IMGB_MAX_PLANE];    /* width of each image plane */
    int      h[SCMN_IMGB_MAX_PLANE];    /* height of each image plane */
    int      s[SCMN_IMGB_MAX_PLANE];    /* stride of each image plane */
    int      e[SCMN_IMGB_MAX_PLANE];    /* elevation of each image plane */
    void   * a[SCMN_IMGB_MAX_PLANE];    /* user space address of each image plane */
    void   * p[SCMN_IMGB_MAX_PLANE];    /* physical address of each image plane, if needs */
    int      cs;    /* color space type of image */
    int      x;    /* left postion, if needs */
    int      y;    /* top position, if needs */
    int      __dummy2;    /* to align memory */
    int      data[16];    /* arbitrary data */

    int fd[SCMN_IMGB_MAX_PLANE];    /* DMABUF fd of each image plane */
    int buf_share_method;

    int y_size;                         /* Y plane size in case of ST12 */
    int uv_size;                        /* UV plane size in case of ST12 */
    //void *bo[SCMN_IMGB_MAX_PLANE];      /* Tizen buffer object of each image plane */
    tbm_bo bo[SCMN_IMGB_MAX_PLANE];

    void *jpeg_data;                    /* JPEG data */
    int jpeg_size;                      /* JPEG size */

    int tz_enable;                      /* tzmem buffer */
} SCMN_IMGB;

#ifdef USE_TBM

#define MFC_INPUT_BUFFER_PLANE      1
#define MFC_OUTPUT_BUFFER_PLANE     2

#define MAX_INPUT_BUFFER            16
#define MAX_OUTPUT_BUFFER           16

typedef struct _TBMBuffer TBMBuffer;
typedef struct _TBMInputBuffer TBMInputBuffer;
typedef struct _TBMOutputBuffer TBMOutputBuffer;

struct _TBMBuffer
{
   OMX_U32 mBufFD;
   tbm_bo  mBo;
   OMX_PTR mPtr;
   OMX_U32 nAllocLen;
};

struct _TBMInputBuffer
{
    struct _TBMBuffer tbmBuffer[MAX_INPUT_BUFFER];
    OMX_U32 allocatedCount;
    GList *buffers;
};

struct _TBMOutputBuffer
{
#ifdef USE_MM_VIDEO_BUFFER
    MMVideoBuffer *tbmBuffer[MAX_OUTPUT_BUFFER];
#else
    SCMN_IMGB *tbmBuffer[MAX_OUTPUT_BUFFER];
#endif
    OMX_U32 allocatedCount;
    GList *buffers;
};

typedef struct _EnableGemBuffersParams EnableGemBuffersParams;

struct _EnableGemBuffersParams
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL enable;
};


#endif

enum
{
    BUF_SHARE_METHOD_PADDR = 0,
    BUF_SHARE_METHOD_FD = 1,
    BUF_SHARE_METHOD_TIZEN_BUFFER = 2,
    BUF_SHARE_METHOD_FLUSH_BUFFER = 3,
}; /* buf_share_method */

/* Extended color formats */
enum {
    OMX_EXT_COLOR_FormatNV12TPhysicalAddress = 0x7F000001, /**< Reserved region for introducing Vendor Extensions */
    OMX_EXT_COLOR_FormatNV12LPhysicalAddress = 0x7F000002,
    OMX_EXT_COLOR_FormatNV12Tiled = 0x7FC00002,
    OMX_EXT_COLOR_FormatNV12TFdValue = 0x7F000012,
    OMX_EXT_COLOR_FormatNV12LFdValue = 0x7F000013
};

#ifdef USE_TBM
/* Extended port settings. */
enum {
    OMX_IndexParamEnablePlatformSpecificBuffers = 0x7F000011
};
#endif

/* modification: Add_component_vendor */
enum GOmxVendor
{
    GOMX_VENDOR_DEFAULT,
    GOMX_VENDOR_SLSI_SEC,
    GOMX_VENDOR_SLSI_EXYNOS,
    GOMX_VENDOR_QCT,
    GOMX_VENDOR_SPRD
};

typedef enum {
  /* Everything good and the buffer is valid */
  GST_OMX_ACQUIRE_BUFFER_OK = 0,
  /* The port is flushing, exit ASAP */
  GST_OMX_ACQUIRE_BUFFER_FLUSHING,
  /* The port must be reconfigured */
  GST_OMX_ACQUIRE_BUFFER_RECONFIGURE,
  /* The port is EOS */
  GST_OMX_ACQUIRE_BUFFER_EOS,
  /* A fatal error happened */
  GST_OMX_ACQUIRE_BUFFER_ERROR
} GstOMXAcquireBufferReturn;

struct _GstOMXCore {
  /* Handle to the OpenMAX IL core shared library */
  GModule *module;

  /* Current number of users, transitions from/to 0
   * call init/deinit */
  GMutex lock;
  gint user_count; /* LOCK */

  /* MODIFICATION */
  GOmxVendor component_vendor; /* to check omx vender */
  gboolean secure; /* trust zone */

  /* OpenMAX core library functions, protected with LOCK */
  OMX_ERRORTYPE (*init) (void);
  OMX_ERRORTYPE (*deinit) (void);
  OMX_ERRORTYPE (*get_handle) (OMX_HANDLETYPE * handle,
      OMX_STRING name, OMX_PTR data, OMX_CALLBACKTYPE * callbacks);
  OMX_ERRORTYPE (*free_handle) (OMX_HANDLETYPE handle);
  OMX_ERRORTYPE (*setup_tunnel) (OMX_HANDLETYPE output, OMX_U32 outport, OMX_HANDLETYPE input, OMX_U32 inport);
};

typedef enum {
  GST_OMX_MESSAGE_STATE_SET,
  GST_OMX_MESSAGE_FLUSH,
  GST_OMX_MESSAGE_ERROR,
  GST_OMX_MESSAGE_PORT_ENABLE,
  GST_OMX_MESSAGE_PORT_SETTINGS_CHANGED,
  GST_OMX_MESSAGE_BUFFER_FLAG,
  GST_OMX_MESSAGE_BUFFER_DONE,
} GstOMXMessageType;

struct _GstOMXMessage {
  GstOMXMessageType type;

  union {
    struct {
      OMX_STATETYPE state;
    } state_set;
    struct {
      OMX_U32 port;
    } flush;
    struct {
      OMX_ERRORTYPE error;
    } error;
    struct {
      OMX_U32 port;
      OMX_BOOL enable;
    } port_enable;
    struct {
      OMX_U32 port;
    } port_settings_changed;
    struct {
      OMX_U32 port;
      OMX_U32 flags;
    } buffer_flag;
    struct {
      OMX_HANDLETYPE component;
      OMX_PTR app_data;
      OMX_BUFFERHEADERTYPE *buffer;
      OMX_BOOL empty;
    } buffer_done;
  } content;
};

struct _GstOMXPort {
  GstOMXComponent *comp;
  guint32 index;

  gboolean tunneled;

  OMX_PARAM_PORTDEFINITIONTYPE port_def;
  GPtrArray *buffers; /* Contains GstOMXBuffer* */
  GQueue pending_buffers; /* Contains GstOMXBuffer* */
  gboolean flushing;
  gboolean flushed; /* TRUE after OMX_CommandFlush was done */
  gboolean enabled_pending;  /* TRUE after OMX_Command{En,Dis}able */
  gboolean disabled_pending; /* was done until it took effect */
  gboolean eos; /* TRUE after a buffer with EOS flag was received */

  /* Increased whenever the settings of these port change.
   * If settings_cookie != configured_settings_cookie
   * the port has to be reconfigured.
   */
  gint settings_cookie;
  gint configured_settings_cookie;
};

struct _GstOMXComponent {
  GstObject *parent;

  gchar *name; /* for debugging mostly */

  OMX_HANDLETYPE handle;
  GstOMXCore *core;

  guint64 hacks; /* Flags, GST_OMX_HACK_* */

  /* Added once, never changed. No locks necessary */
  GPtrArray *ports; /* Contains GstOMXPort* */
  gint n_in_ports, n_out_ports;

  /* Locking order: lock -> messages_lock
   *
   * Never hold lock while waiting for messages_cond
   * Always check that messages is empty before waiting */
  GMutex lock;

  GQueue messages; /* Queue of GstOMXMessages */
  GMutex messages_lock;
  GCond messages_cond;

  OMX_STATETYPE state;
  /* OMX_StateInvalid if no pending state */
  OMX_STATETYPE pending_state;
  /* OMX_ErrorNone usually, if different nothing will work */
  OMX_ERRORTYPE last_error;

  GList *pending_reconfigure_outports;
};

struct _GstOMXBuffer {
  GstOMXPort *port;
  OMX_BUFFERHEADERTYPE *omx_buf;

  /* TRUE if the buffer is used by the port, i.e.
   * between {Empty,Fill}ThisBuffer and the callback
   */
  gboolean used;

  /* Cookie of the settings when this buffer was allocated */
  gint settings_cookie;

  /* TRUE if this is an EGLImage */
  gboolean eglimage;

#ifdef USE_TBM
#ifdef USE_MM_VIDEO_BUFFER
  /* MMVideoBuffer array to use TBM buffers */
   MMVideoBuffer *scmn_buffer;
#else
  /* SCMN_IMGB array to use TBM buffers */
  SCMN_IMGB *scmn_buffer;
#endif
#endif
};

struct _GstOMXClassData {
  const gchar *core_name;
  const gchar *component_name;
  const gchar *component_role;

  const gchar *default_src_template_caps;
  const gchar *default_sink_template_caps;

  guint32 in_port_index, out_port_index;

  guint64 hacks;
};

GKeyFile *        gst_omx_get_configuration (void);

const gchar *     gst_omx_error_to_string (OMX_ERRORTYPE err);
const gchar *     gst_omx_state_to_string (OMX_STATETYPE state);
const gchar *     gst_omx_command_to_string (OMX_COMMANDTYPE cmd);

guint64           gst_omx_parse_hacks (gchar ** hacks);

GstOMXCore *      gst_omx_core_acquire (const gchar * filename);
void              gst_omx_core_release (GstOMXCore * core);


GstOMXComponent * gst_omx_component_new (GstObject * parent, const gchar *core_name, const gchar *component_name, const gchar * component_role, guint64 hacks);
void              gst_omx_component_free (GstOMXComponent * comp);

OMX_ERRORTYPE     gst_omx_component_set_state (GstOMXComponent * comp, OMX_STATETYPE state);
OMX_STATETYPE     gst_omx_component_get_state (GstOMXComponent * comp, GstClockTime timeout);

OMX_ERRORTYPE     gst_omx_component_get_last_error (GstOMXComponent * comp);
const gchar *     gst_omx_component_get_last_error_string (GstOMXComponent * comp);

GstOMXPort *      gst_omx_component_add_port (GstOMXComponent * comp, guint32 index);
GstOMXPort *      gst_omx_component_get_port (GstOMXComponent * comp, guint32 index);

OMX_ERRORTYPE     gst_omx_component_get_parameter (GstOMXComponent * comp, OMX_INDEXTYPE index, gpointer param);
OMX_ERRORTYPE     gst_omx_component_set_parameter (GstOMXComponent * comp, OMX_INDEXTYPE index, gpointer param);

OMX_ERRORTYPE     gst_omx_component_get_config (GstOMXComponent * comp, OMX_INDEXTYPE index, gpointer config);
OMX_ERRORTYPE     gst_omx_component_set_config (GstOMXComponent * comp, OMX_INDEXTYPE index, gpointer config);
OMX_ERRORTYPE     gst_omx_component_setup_tunnel (GstOMXComponent * comp1, GstOMXPort * port1, GstOMXComponent * comp2, GstOMXPort * port2);
OMX_ERRORTYPE     gst_omx_component_close_tunnel (GstOMXComponent * comp1, GstOMXPort * port1, GstOMXComponent * comp2, GstOMXPort * port2);


OMX_ERRORTYPE     gst_omx_port_get_port_definition (GstOMXPort * port, OMX_PARAM_PORTDEFINITIONTYPE * port_def);
OMX_ERRORTYPE     gst_omx_port_update_port_definition (GstOMXPort *port, OMX_PARAM_PORTDEFINITIONTYPE *port_definition);

GstOMXAcquireBufferReturn gst_omx_port_acquire_buffer (GstOMXPort *port, GstOMXBuffer **buf);
OMX_ERRORTYPE     gst_omx_port_release_buffer (GstOMXPort *port, GstOMXBuffer *buf);

OMX_ERRORTYPE     gst_omx_port_set_flushing (GstOMXPort *port, GstClockTime timeout, gboolean flush);
gboolean          gst_omx_port_is_flushing (GstOMXPort *port);

OMX_ERRORTYPE     gst_omx_port_allocate_buffers (GstOMXPort *port);
#ifdef USE_TBM
OMX_ERRORTYPE     gst_omx_port_tbm_allocate_dec_buffers (tbm_bufmgr  bufMgr, GstOMXPort * port, int eCompressionFormat);
OMX_ERRORTYPE     gst_omx_port_tbm_allocate_enc_buffers (tbm_bufmgr  bufMgr, GstOMXPort * port, int eCompressionFormat);
#endif
OMX_ERRORTYPE     gst_omx_port_use_buffers (GstOMXPort *port, const GList *buffers);
OMX_ERRORTYPE     gst_omx_port_use_eglimages (GstOMXPort *port, const GList *images);
OMX_ERRORTYPE     gst_omx_port_deallocate_buffers (GstOMXPort *port);
OMX_ERRORTYPE     gst_omx_port_populate (GstOMXPort *port);
OMX_ERRORTYPE     gst_omx_port_wait_buffers_released (GstOMXPort * port, GstClockTime timeout);

OMX_ERRORTYPE     gst_omx_port_mark_reconfigured (GstOMXPort * port);

OMX_ERRORTYPE     gst_omx_port_set_enabled (GstOMXPort * port, gboolean enabled);
OMX_ERRORTYPE     gst_omx_port_wait_enabled (GstOMXPort * port, GstClockTime timeout);
gboolean          gst_omx_port_is_enabled (GstOMXPort * port);


void              gst_omx_set_default_role (GstOMXClassData *class_data, const gchar *default_role);

#ifdef USE_TBM

/*MFC Buffer alignment macros*/
#define S5P_FIMV_DEC_BUF_ALIGN                  (8 * 1024)
#define S5P_FIMV_ENC_BUF_ALIGN                  (8 * 1024)
#define S5P_FIMV_NV12M_HALIGN                   16
#define S5P_FIMV_NV12M_LVALIGN                  16
#define S5P_FIMV_NV12M_CVALIGN                  8
#define S5P_FIMV_NV12MT_HALIGN                  128
#define S5P_FIMV_NV12MT_VALIGN                  64
#define S5P_FIMV_NV12M_SALIGN                   2048
#define S5P_FIMV_NV12MT_SALIGN                  8192

#define ALIGN(x, a)       (((x) + (a) - 1) & ~((a) - 1))

/* Buffer alignment defines */
#define SZ_1M                                   0x00100000
#define S5P_FIMV_D_ALIGN_PLANE_SIZE             64

#define S5P_FIMV_MAX_FRAME_SIZE                 (2 * SZ_1M)
#define S5P_FIMV_NUM_PIXELS_IN_MB_ROW           16
#define S5P_FIMV_NUM_PIXELS_IN_MB_COL           16

/* Macro */
#define ALIGN_TO_4KB(x)   ((((x) + (1 << 12) - 1) >> 12) << 12)
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define CHOOSE_MAX_SIZE(a,b) ((a) > (b) ? (a) : (b))

int new_calc_plane(int width, int height);
int new_calc_yplane(int width, int height);
int new_calc_uvplane(int width, int height);

int calc_plane(int width, int height);
int calc_yplane(int width, int height);
int calc_uvplane(int width, int height);
int gst_omx_calculate_y_size(int compressionFormat, int width, int height);
int gst_omx_calculate_uv_size(int compressionFormat, int width, int height);

tbm_bo            gst_omx_tbm_allocate_bo(tbm_bufmgr hBufmgr, int size);
void              gst_omx_tbm_deallocate_bo(tbm_bo bo);
OMX_U32           gst_omx_tbm_get_bo_fd(tbm_bo bo);
OMX_PTR           gst_omx_tbm_get_bo_ptr(tbm_bo bo);

#endif


G_END_DECLS

#endif /* __GST_OMX_H__ */
