#include <gstreamermm.h>
#include <glibmm/main.h>
#include <glibmm/convert.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>

#include "hud.hpp"

Glib::RefPtr<Glib::MainLoop> mainloop;
SimHUD* hud = NULL;

// This function is used to receive asynchronous messages in the main loop.
bool on_bus_message(const Glib::RefPtr<Gst::Bus>& bus ,
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
      if (hud) hud->bus_message(bus, message);
      break;
  }

  return true;
}

int main(int argc, char** argv)
{
  Glib::RefPtr<Gst::Pipeline> pipeline;
  Glib::RefPtr<Gst::Element> e_source;
  Glib::RefPtr<Gst::Element> e_hud;
  Glib::RefPtr<Gst::Element> e_cvt;
  Glib::RefPtr<Gst::Element> e_sink;

  // Format filters
  Glib::RefPtr<Gst::Caps> caps_informat;
  Glib::RefPtr<Gst::Caps> caps_outformat;

  // Initialize Gstreamermm:
  Gst::init(argc, argv);

  // Create pipeline:
  pipeline = Gst::Pipeline::create("simhud-pipeline");

  hud = new SimHUD();

  // Create elements:
  e_source = Gst::ElementFactory::create_element("videotestsrc");
  e_hud = hud->element();
  e_cvt = Gst::ElementFactory::create_element("videoconvert");
  e_sink = Gst::ElementFactory::create_element("autovideosink");

  // Define filtering capabilities to force video formats
  caps_informat  = Gst::Caps::create_simple("video/x-raw");
  caps_informat->set_value("width", 1920);
  caps_informat->set_value("height", 1080);

  caps_outformat = Gst::Caps::create_simple("video/x-raw");
  caps_informat->set_value("width", 1280);
  caps_informat->set_value("height", 720);


  // We must add the elements to the pipeline before linking them:
  try
  {
    pipeline
      ->add(e_source)
      ->add(e_hud)
      ->add(e_cvt)
      ->add(e_sink);
  }
  catch (std::runtime_error& ex)
  {
    std::cerr << "Exception while adding: " << ex.what() << std::endl;
    return 1;
  }

  // Link the elements together:
  try
  {
    e_source
      ->link(e_hud, caps_informat)
      ->link(e_cvt)
      ->link(e_sink, caps_outformat);
  }
  catch(const std::runtime_error& error)
  {
    std::cerr << "Exception while linking: " << error.what() << std::endl;
  }



  // Create the main loop.
  mainloop = Glib::MainLoop::create();

  // Add a bus watcher and also allow the HUD to listen to bus events
  Glib::RefPtr<Gst::Bus> bus = pipeline->get_bus();
  //bus->add_watch(sigc::mem_fun(hud, &SimHUD::bus_message));
  bus->add_watch(sigc::ptr_fun(&on_bus_message));

  // Let the HUD figure out frame size and stuff
  std::cout << "Setting to PAUSED." << std::endl;
  pipeline->set_state(Gst::STATE_PAUSED);

  // Go!
  std::cout << "Setting to PLAYING." << std::endl;
  pipeline->set_state(Gst::STATE_PLAYING);

  std::cout << "Running." << std::endl;
  mainloop->run();

  // Clean up nicely:
  std::cout << "Returned. Setting state to NULL." << std::endl;
  pipeline->set_state(Gst::STATE_NULL);



  return 0;
}

