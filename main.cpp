#include <gstreamermm.h>
#include <glibmm/main.h>
#include <glibmm/convert.h>
#include <stdlib.h>
#include <iostream>

#include "hud.hpp"

Glib::RefPtr<Glib::MainLoop> mainloop;

// This function is used to receive asynchronous messages in the main loop.
bool on_bus_message(const Glib::RefPtr<Gst::Bus>& /* bus */,
    const Glib::RefPtr<Gst::Message>& message)
{
  switch(message->get_message_type()) {
    case Gst::MESSAGE_EOS:
      std::cout << std::endl << "End of stream" << std::endl;
      mainloop->quit();
      return false;
    case Gst::MESSAGE_ERROR:
      {


        Glib::RefPtr<Gst::MessageError> msgError =
          Glib::RefPtr<Gst::MessageError>::cast_static(message);

        if(msgError)
        {
          Glib::Error err;
          err = msgError->parse();
          std::string debug;
          debug = msgError->parse_debug();
          std::cerr << "Error: " << err.what() << std::endl;
          std::cerr << "       " << debug << std::endl;
        }
        else
          std::cerr << "Error." << std::endl;

        mainloop->quit();
        return false;
      }
    default:
      break;
  }

  return true;
}

int main(int argc, char** argv)
{
  Glib::RefPtr<Gst::Pipeline> pipeline;
  Glib::RefPtr<Gst::Element> element_source, element_hud, element_cvt, element_sink;

  // Initialize Gstreamermm:
  Gst::init(argc, argv);

  // Create pipeline:
  pipeline = Gst::Pipeline::create("my-pipeline");

  SimHUD hud;

  // Create elements:
  element_source = Gst::ElementFactory::create_element("videotestsrc");
  element_hud = hud.element();
  element_cvt = Gst::ElementFactory::create_element("videoconvert");
  element_sink = Gst::ElementFactory::create_element("autovideosink");

  // We must add the elements to the pipeline before linking them:
  try
  {
    pipeline->add(element_source)->add(element_hud)->add(element_cvt)->add(element_sink);
  }
  catch (std::runtime_error& ex)
  {
    std::cerr << "Exception while adding: " << ex.what() << std::endl;
    return 1;
  }

  // Link the elements together:
  try
  {
    element_source->link(element_hud)->link(element_cvt)->link(element_sink);
  }
  catch(const std::runtime_error& error)
  {
    std::cerr << "Exception while linking: " << error.what() << std::endl;
  }



  // Create the main loop.
  mainloop = Glib::MainLoop::create();

  // Get the bus from the playbin, and add a bus watch to the default main
  // context with the default priority:
  Glib::RefPtr<Gst::Bus> bus = pipeline->get_bus();
  bus->add_watch(sigc::ptr_fun(&on_bus_message));

  std::cout << "Setting to PLAYING." << std::endl;
  pipeline->set_state(Gst::STATE_PLAYING);

  std::cout << "Running." << std::endl;
  mainloop->run();

  // Clean up nicely:
  std::cout << "Returned. Setting state to NULL." << std::endl;
  pipeline->set_state(Gst::STATE_NULL);



  return 0;
}

