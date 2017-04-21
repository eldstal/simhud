#pragma once

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
  Elm e_leftbox;
  Elm e_rightbox;
  Elm e_bottombox;

  // Set text properties, shading, etc.
  Elm createTextElement(int horizontal, double ypos);
  void setupTextElement(Elm &element, int horizontal, double ypos);

};



