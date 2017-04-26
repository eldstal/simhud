#pragma once

#include <gst/video/video.h>
#include <gstreamermm.h>
#include <glibmm/refptr.h>


class SimHUD {
  typedef Glib::RefPtr<Gst::Element> Elm;

public:

  SimHUD();

  /*
   * Return a Gstreamer filter element which applies a HUD
   * to a video stream
   */
  Glib::RefPtr<Gst::Bin> element();


private:
  Glib::RefPtr<Gst::Bin> bin;
  Elm e_timestamp;
  Elm e_overlay;

  // Properties of the video stream
  // We're forced to revert to the C api by the cairo overlay
  // ...sorry about that.
  bool info_valid;
  GstVideoInfo* stream_info;

  // Set text properties, shading, etc.
  Elm createTextElement(int horizontal, double ypos);
  void setupTextElement(Elm &element, int horizontal, double ypos);

  // Callbacks from the cairo overlay
  static void prepare_overlay(GstElement* overlay, GstCaps* caps, gpointer hud);

  static void draw_overlay(GstElement* overlay, cairo_t * cr, guint64 timestamp, guint64 duration, gpointer hud);
};



