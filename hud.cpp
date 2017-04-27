#include <gstreamermm.h>
#include <iostream>
#include <inttypes.h>
#include <cairomm/cairomm.h>

#include <pangomm.h>
#include <pangomm/tabarray.h>
#include <pango/pangocairo.h>

#include <sstream>
#include <string>
#include <iomanip>


#include "hud.hpp"

using namespace Glib;
using namespace Gst;
using std::string;
using std::cerr;
using std::endl;

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

void SimHUD::set_values(const SensorValues& v) {
  // TODO: locking
  this->sensors = v;
}

void SimHUD::setupTextElement(Elm& element, int horizontal, double ypos) {

  // Appearance
  element->set_property<ustring>("font-desc", "Calibri Light 16");

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
  Cairo::RefPtr<Cairo::Context> cairo = Cairo::Context::create(c.get_target());

  int w = GST_VIDEO_INFO_WIDTH(self->stream_info);
  int h = GST_VIDEO_INFO_HEIGHT(self->stream_info);

  //
  // Layout parameters
  //
  int margin = 10;
  int padding = 10;
  float shading = 0.4;

  // A rendering context for all the text on the screen
  RefPtr<Pango::Context> ctx = Glib::wrap(pango_cairo_create_context(cr));
  Pango::FontDescription font("Calibri Light 16");
  ctx->set_font_description(font);

  //
  // A "paragraph" of text on the left-hand edge of the screen
  //
  RefPtr<Pango::Layout> left_text = Pango::Layout::create(ctx);

  int colwidth=w/8;
  colwidth = colwidth > 120 ? 120 : colwidth;
  Pango::TabArray tabs(1,true);
  tabs.set_tab(0, Pango::TabAlign::TAB_LEFT, colwidth);
  left_text->set_tabs(tabs);

  // TODO: Locking
  SensorValues sens = self->sensors;

  //
  // Generate left pane text
  //
  std::ostringstream text;
  text
       << std::setprecision(4)
       << "Depth:\t" << sens.depth << "m\n"
       << "\n"
       << "Voltage:\t" << sens.depth << "V\n"
       << "Current:\t" << sens.depth << "A\n"
       << "\n"
       << "Ext. Temp:\t" << sens.temp_external << "°C\n"
       << "Int. Temp:\t" << sens.internal.temp << "°C\n"
       << "Int. Humidity:\t" << sens.internal.temp << "%\n"
  ;


  left_text->set_text(text.str());



  //
  // Position elements on the canvas
  //
  int tw, th;
  left_text->get_pixel_size(tw, th);
  int tx = margin;
  int ty = h - (margin + th + 2*padding);
  cairo->set_source_rgba(0.0, 0.0, 0.0, shading);
  cairo->rectangle(tx, ty, tw+2*padding, th+2*padding);
  cairo->fill();
  cairo->move_to(tx+padding, ty+padding);
  cairo->set_source_rgb(0.8, 0.8, 0.8);
  left_text->show_in_cairo_context(cairo);


  cairo->stroke();
}
