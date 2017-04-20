#include <gstreamermm.h>
#include <iostream>


#include "hud.hpp"

using namespace Glib;
using namespace Gst;

SimHUD::SimHUD() {
  this->bin = Gst::Bin::create("sim-hud");

  RefPtr<Element> identity = ElementFactory::create_element("identity");


  try {
    bin->add(identity);

  } catch (const std::runtime_error& ex) {
    std::cerr << "Exception while adding: " << ex.what() << std::endl;
  }

  // Map out the sink (input) and source (output) of our HUD filter
  bin->add_ghost_pad(identity, "sink", "sink");
  bin->add_ghost_pad(identity, "src", "src");

}



Glib::RefPtr<Gst::Bin> SimHUD::element() {
  return this->bin;
}
