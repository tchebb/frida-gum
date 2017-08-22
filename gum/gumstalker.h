/*
 * Copyright (C) 2009-2017 Ole André Vadla Ravnås <oleavr@nowsecure.com>
 * Copyright (C)      2010 Karl Trygve Kalleberg <karltk@boblycat.org>
 *
 * Licence: wxWindows Library Licence, Version 3.1
 */

#ifndef __GUM_STALKER_H__
#define __GUM_STALKER_H__

#include <capstone.h>
#include <glib-object.h>
#include <gum/arch-x86/gumx86writer.h>
#include <gum/arch-arm/gumarmwriter.h>
#include <gum/arch-arm/gumthumbwriter.h>
#include <gum/arch-arm64/gumarm64writer.h>
#include <gum/arch-mips/gummipswriter.h>
#include <gum/gumdefs.h>
#include <gum/gumeventsink.h>
#include <gum/gumprocess.h>

#define GUM_TYPE_STALKER (gum_stalker_get_type ())
#define GUM_STALKER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
    GUM_TYPE_STALKER, GumStalker))
#define GUM_STALKER_CAST(obj) ((GumStalker *) (obj))
#define GUM_STALKER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
    GUM_TYPE_STALKER, GumStalkerClass))
#define GUM_IS_STALKER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
    GUM_TYPE_STALKER))
#define GUM_IS_STALKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE (\
    (klass), GUM_TYPE_STALKER))
#define GUM_STALKER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS (\
    (obj), GUM_TYPE_STALKER, GumStalkerClass))

#define GUM_TYPE_STALKER_TRANSFORMER (gum_stalker_transformer_get_type ())
#define GUM_STALKER_TRANSFORMER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
    GUM_TYPE_STALKER_TRANSFORMER, GumStalkerTransformer))
#define GUM_IS_STALKER_TRANSFORMER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
    GUM_TYPE_STALKER_TRANSFORMER))
#define GUM_STALKER_TRANSFORMER_GET_INTERFACE(inst) (\
    G_TYPE_INSTANCE_GET_INTERFACE ((inst), GUM_TYPE_STALKER_TRANSFORMER,\
    GumStalkerTransformerIface))

#define GUM_TYPE_DEFAULT_STALKER_TRANSFORMER \
    (gum_default_stalker_transformer_get_type ())
G_DECLARE_FINAL_TYPE (GumDefaultStalkerTransformer,
    gum_default_stalker_transformer, GUM, DEFAULT_STALKER_TRANSFORMER,
    GObject)

#define GUM_TYPE_CALLBACK_STALKER_TRANSFORMER \
    (gum_callback_stalker_transformer_get_type ())
G_DECLARE_FINAL_TYPE (GumCallbackStalkerTransformer,
    gum_callback_stalker_transformer, GUM, CALLBACK_STALKER_TRANSFORMER,
    GObject)

G_BEGIN_DECLS

typedef struct _GumStalker           GumStalker;
typedef struct _GumStalkerClass      GumStalkerClass;
typedef struct _GumStalkerPrivate    GumStalkerPrivate;

typedef struct _GumStalkerTransformer GumStalkerTransformer;
typedef struct _GumStalkerTransformerIface GumStalkerTransformerIface;
typedef struct _GumStalkerIterator GumStalkerIterator;
typedef union _GumStalkerWriter GumStalkerWriter;
typedef void (* GumStalkerTransformerCallback) (GumStalkerIterator * iterator,
    GumStalkerWriter * output, gpointer user_data);
typedef void (* GumStalkerCallout) (GumCpuContext * cpu_context,
    gpointer user_data);

typedef guint GumProbeId;
typedef struct _GumCallSite GumCallSite;
typedef void (* GumCallProbeCallback) (GumCallSite * site, gpointer user_data);

struct _GumStalker
{
  GObject parent;

  GumStalkerPrivate * priv;
};

struct _GumStalkerClass
{
  GObjectClass parent_class;
};

struct _GumStalkerTransformerIface
{
  GTypeInterface parent;

  void (* transform_block) (GumStalkerTransformer * self,
      GumStalkerIterator * iterator, GumStalkerWriter * output);
};

union _GumStalkerWriter
{
  GumX86Writer x86;
  GumArmWriter arm;
  GumThumbWriter thumb;
  GumArm64Writer arm64;
  GumMipsWriter mips;
};

struct _GumCallSite
{
  gpointer block_address;
  gpointer stack_data;
  GumCpuContext * cpu_context;
};

GUM_API GType gum_stalker_get_type (void) G_GNUC_CONST;

GUM_API GumStalker * gum_stalker_new (void);

GUM_API void gum_stalker_exclude (GumStalker * self,
    const GumMemoryRange * range);

GUM_API gint gum_stalker_get_trust_threshold (GumStalker * self);
GUM_API void gum_stalker_set_trust_threshold (GumStalker * self,
    gint trust_threshold);

GUM_API void gum_stalker_stop (GumStalker * self);
GUM_API gboolean gum_stalker_garbage_collect (GumStalker * self);

GUM_API void gum_stalker_follow_me (GumStalker * self,
    GumStalkerTransformer * transformer, GumEventSink * sink);
GUM_API void gum_stalker_unfollow_me (GumStalker * self);
GUM_API gboolean gum_stalker_is_following_me (GumStalker * self);

GUM_API void gum_stalker_follow (GumStalker * self, GumThreadId thread_id,
    GumStalkerTransformer * transformer, GumEventSink * sink);
GUM_API void gum_stalker_unfollow (GumStalker * self, GumThreadId thread_id);

GUM_API GumProbeId gum_stalker_add_call_probe (GumStalker * self,
    gpointer target_address, GumCallProbeCallback callback, gpointer data,
    GDestroyNotify notify);
GUM_API void gum_stalker_remove_call_probe (GumStalker * self,
    GumProbeId id);

#define gum_call_site_get_nth_argument(s, n) \
    gum_cpu_context_get_nth_argument ((s)->cpu_context, n)
#define gum_call_site_replace_nth_argument(s, n, v) \
    gum_cpu_context_replace_nth_argument ((s)->cpu_context, n, v)

GType gum_stalker_transformer_get_type (void);

GUM_API GumStalkerTransformer * gum_stalker_transformer_make_default (void);
GUM_API GumStalkerTransformer * gum_stalker_transformer_make_from_callback (
    GumStalkerTransformerCallback callback, gpointer data,
    GDestroyNotify data_destroy);

GUM_API void gum_stalker_transformer_transform_block (
    GumStalkerTransformer * self, GumStalkerIterator * iterator,
    GumStalkerWriter * output);

GUM_API gboolean gum_stalker_iterator_next (GumStalkerIterator * self,
    const cs_insn ** insn);
GUM_API void gum_stalker_iterator_keep (GumStalkerIterator * self);
GUM_API void gum_stalker_iterator_put_callout (GumStalkerIterator * self,
    GumStalkerCallout callout, gpointer data, GDestroyNotify data_destroy);

G_END_DECLS

#endif
