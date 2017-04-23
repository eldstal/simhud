#pragma once

#include <gstreamermm.h>
#include <glibmm/refptr.h>
#include <gdkmm/pixbuf.h>


class SimHUD {
  typedef Glib::RefPtr<Gst::Element> Elm;

public:

  SimHUD();

  /*
   * Return a Gstreamer filter element which applies a HUD
   * to a video stream
   */
  Glib::RefPtr<Gst::Bin> element();

  /*
   * There has been a capability update,
   * reconfigure.
   */
  void reconfigure();

  bool bus_message(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message);


private:
  Glib::RefPtr<Gst::Bin> bin;
  Elm e_timestamp;
  Elm e_leftbox;
  Elm e_rightbox;
  Elm e_bottombox;
  Elm e_overlay;

  // An image of the same size as the video stream
  // We do double buffering to be able to modify in peace
  // front is the one currently being displayed, back is safe to draw on.
  Glib::RefPtr<Gdk::Pixbuf> overlay_front;
  Glib::RefPtr<Gdk::Pixbuf> overlay_back;

  // Set text properties, shading, etc.
  Elm createTextElement(int horizontal, double ypos);
  void setupTextElement(Elm &element, int horizontal, double ypos);

  // Draw the HUD onto the overlay image
  void redraw();
};



