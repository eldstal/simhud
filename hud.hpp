#pragma once

#include <gstreamermm.h>
#include <glibmm/refptr.h>

class SimHUD {

public:

  SimHUD();

  /*
   * Return a Gstreamer filter element which applies a HUD
   * to a video stream
   */
  Glib::RefPtr<Gst::Bin> element();


private:
  Glib::RefPtr<Gst::Bin> bin;

}



