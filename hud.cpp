#include <gstreamermm.h>
#include <iostream>
#include <inttypes.h>


#include "hud.hpp"

using namespace Glib;
using namespace Gdk;
using namespace Gst;
using std::cerr;
using std::endl;

SimHUD::SimHUD() {
  bin = Gst::Bin::create("sim-hud");

  RefPtr<Element> identity = ElementFactory::create_element("identity");

  e_timestamp = ElementFactory::create_element("clockoverlay");
  setupTextElement(e_timestamp, 0, 0.0);
  e_timestamp->set_property<ustring>("time-format", "%F %T");

  e_leftbox = createTextElement(-1, 1.0);
  e_bottombox = createTextElement(0, 1.0);
  e_rightbox = createTextElement(1, 1.0);

  e_leftbox->set_property<ustring>("text", "Line one\nLine two");
  e_bottombox->set_property<ustring>("text", "Just one line...");
  e_rightbox->set_property<ustring>("text", "Line one\nLine two");

  e_overlay = ElementFactory::create_element("gdkpixbufoverlay");


  try {
    bin
      ->add(identity)
      ->add(e_timestamp)
      ->add(e_leftbox)
      ->add(e_bottombox)
      ->add(e_rightbox)
      ->add(e_overlay);

    identity
      ->link(e_timestamp)
      ->link(e_leftbox)
      ->link(e_bottombox)
      ->link(e_rightbox)
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

void SimHUD::reconfigure() {
  RefPtr<Gst::Pad> input = bin->get_static_pad("sink");
  if (!input) {
    cerr << "Unable to reconfigure HUD: No pad found." << endl;
    return;
  }

  RefPtr<Gst::Caps> format = input->get_current_caps();
  if (!format) {
    cerr << "Unable to reconfigure HUD: No caps found." << endl;
    return;
  }

  const Gst::Structure strct = format->get_structure(0);

  Glib::Value<int32_t> width, height;

  strct.get_field("width", width);
  strct.get_field("height", height);

  int32_t w = width.get();
  int32_t h = height.get();

  w = 10;
  h = 10;

  if (!this->overlay_back ||
      this->overlay_back->get_width() != w ||
      this->overlay_back->get_height() != h) {
    cerr << "Reconfiguring for stream size " << w << "x" << h << endl;

    // Create a new pixel buffer pair
    this->overlay_back = Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, 1, 8, w, h);
    this->overlay_front = Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, 1, 8, w, h);

    redraw();
  }
}



bool SimHUD::bus_message(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message) {
  switch(message->get_message_type()) {
    case Gst::MESSAGE_STATE_CHANGED:
      reconfigure();
      break;
    default:
      break;
  }
  return true;

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


void SimHUD::redraw() {

    if (!overlay_back ||
        !overlay_front) {
      cerr << "Null draw buffer. Not drawing the HUD." << endl;
      return;
    }

    //this->overlay_front.swap(this->overlay_back);

    // Swap the drawn buffer into the overlay filter for actual drawing onto the video stream
    //this->e_overlay->set_property< GdkPixbuf* >("pixbuf", this->overlay_front->gobj());
}
