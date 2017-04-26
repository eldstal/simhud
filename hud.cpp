#include <gstreamermm.h>
#include <iostream>
#include <inttypes.h>
#include <cairomm/cairomm.h>


#include "hud.hpp"

using namespace Glib;
using namespace Gst;
using std::cerr;
using std::endl;

// Used for the cairooverlay draw function
typedef struct {
  gboolean valid;
  int width;
  int height;
} CairoOverlayState;

SimHUD::SimHUD() {
  bin = Gst::Bin::create("sim-hud");

  RefPtr<Element> identity = ElementFactory::create_element("identity");

  e_timestamp = ElementFactory::create_element("clockoverlay");
  setupTextElement(e_timestamp, 0, 0.0);
  e_timestamp->set_property<ustring>("time-format", "%F %T");

  // gstreamermm doesn't have wrapers for these signal functions. We use the C API.
  e_overlay = ElementFactory::create_element("cairooverlay");
  this->info_valid = false;
  this->stream_info = gst_video_info_new();
  g_signal_connect(e_overlay->gobj(),
                   "caps-changed",
                   G_CALLBACK(SimHUD::prepare_overlay),
                   this);
  g_signal_connect(e_overlay->gobj(),
                   "draw",
                   G_CALLBACK(SimHUD::draw_overlay),
                   this);


  try {
    bin
      ->add(identity)
      ->add(e_timestamp)
      ->add(e_overlay);

    identity
      ->link(e_timestamp)
      ->link(e_overlay);

  } catch (const std::runtime_error& ex) {
    std::cerr << "Exception while adding: " << ex.what() << std::endl;
  }

  // Map out the sink (input) and source (output) of our HUD filter
  bin->add_ghost_pad(identity, "sink", "sink");
  bin->add_ghost_pad(e_overlay, "src", "src");

}



RefPtr<Gst::Bin> SimHUD::element() {
  return this->bin;
}

void SimHUD::setupTextElement(Elm& element, int horizontal, double ypos) {

  // Appearance
  element->set_property<ustring>("font-desc", "Monospace Light 16");

  element->set_property<bool>("auto-resize", false);

  element->set_property<bool>("draw-outline", false);
  element->set_property<uint32_t>("outline-color", 0xFF000000);

  element->set_property<bool>("draw-shadow", false);

  element->set_property<bool>("shaded-background", true);
  element->set_property<uint8_t>("shading-value", 120);


  // Positioning
  int padding = 5;
  element->set_property<uint32_t>("halignment", 4);   // Absolute, clamped
  element->set_property<uint32_t>("valignment", 3);   // Absolute, clamped

  element->set_property<uint32_t>("y-absolute", ypos);

  if (horizontal < 0) {
    element->set_property<gdouble>("x-absolute", 0);
    element->set_property<uint32_t>("line-alignment", 0);  // Left
    element->set_property<uint32_t>("deltax", padding);
  }

  if (horizontal == 0) {
    element->set_property<gdouble>("x-absolute", 0.5);
    element->set_property<uint32_t>("line-alignment", 1);  // Center
  }

  if (horizontal > 0) {
    element->set_property<gdouble>("x-absolute", 1);
    element->set_property<uint32_t>("line-alignment", 2);  // Right
    element->set_property<uint32_t>("deltax", -padding);
  }

  if (ypos == 0) {
    element->set_property<uint32_t>("deltay", padding);
  }

  if (ypos == 1.0) {
    element->set_property<uint32_t>("deltay", -padding);
  }
}

SimHUD::Elm SimHUD::createTextElement(int horizontal, double ypos) {
  Elm ret = ElementFactory::create_element("textoverlay");
  setupTextElement(ret, horizontal, ypos);
  return ret;
}


void SimHUD::prepare_overlay(GstElement* overlay, GstCaps* caps, gpointer hud) {
  SimHUD* self = (SimHUD*) hud;

  GstVideoInfo info;

  // Save the video settings so that draw_overlay() has
  // dimensions to work with
  if (!gst_video_info_from_caps(self->stream_info, caps)) {
    cerr << "Warning: Invalid capabilities!" << endl;
  } else {
    self->info_valid = true;
  }

}

void SimHUD::draw_overlay(GstElement* overlay, cairo_t * cr, guint64 timestamp, guint64 duration, gpointer hud) {
  SimHUD* self = (SimHUD*) hud;

  if (!self->info_valid) {
    cerr << "Attempted to draw before video stream is identified. Not drawing HUD." << endl;
    return;
  }

  Cairo::Context c(cr);

  int w = GST_VIDEO_INFO_WIDTH(self->stream_info);
  int h = GST_VIDEO_INFO_HEIGHT(self->stream_info);

  


  c.set_source_rgb(0.0, 0.0, 0.0);
  c.rectangle(0, 0, 300, 300);
  c.stroke();

  c.set_source_rgb(0.7, 0.3, 0.3);
  c.fill();
}
