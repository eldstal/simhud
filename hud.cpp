#include <gstreamermm.h>


#include "hud.hpp"


SimHUD::SimHUD() {
  this->bin = Gst::Bin::create("sim-hud");

  Glib::RefPtr<Gst::Element> identity = Gst::ElementFactory::create_element("identity");


  // Map out the sink (input) and source (output) of our HUD filter
}



Glib::RefPtr<Gst::Bin> SimHUD::element() {
  return this->bin;
}
