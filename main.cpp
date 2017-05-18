#include <gstreamermm.h>
#include <glibmm/main.h>
#include <glibmm/convert.h>
#include <gtkmm/application.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>

#include "hud.hpp"


using std::cout;
using std::cerr;
using std::endl;
using std::string;


Glib::RefPtr<Glib::MainLoop> mainloop;
SimHUD* hud = NULL;

bool stop;

// This function is used to receive asynchronous messages in the main loop.
bool on_bus_message(const Glib::RefPtr<Gst::Bus>& bus ,
    const Glib::RefPtr<Gst::Message>& message)
{
  switch(message->get_message_type()) {
    case Gst::MESSAGE_EOS:
      cout << std::endl << "End of stream" << endl;
      mainloop->quit();
      stop = true;
      return false;
    case Gst::MESSAGE_ERROR:
      {


        Glib::RefPtr<Gst::MessageError> msgError =
          Glib::RefPtr<Gst::MessageError>::cast_static(message);

        if(msgError)
        {
          Glib::Error err;
          err = msgError->parse();
          string debug;
          debug = msgError->parse_debug();
          cerr << "Error: " << err.what() << endl;
          cerr << "       " << debug << endl;
        }
        else
          cerr << "Error." << endl;

        mainloop->quit();
        stop = true;
        return false;
      }
  }

  return true;
}

Glib::RefPtr<Gst::Bin> dummy_webcam() {
  Glib::RefPtr<Gst::Bin> ret = Gst::Bin::create("camera");
  Glib::RefPtr<Gst::Caps> caps_capformat;

  Glib::RefPtr<Gst::Element> src = Gst::ElementFactory::create_element("videotestsrc");
  src->set_property<int32_t>("pattern", 0);   // 18=Ball, 0=bars

  Glib::RefPtr<Gst::Element> conv = Gst::ElementFactory::create_element("videoconvert");

  // Define filtering capabilities to force video formats
  caps_capformat  = Gst::Caps::create_simple("video/x-raw");
  caps_capformat->set_value("framerate", Gst::Fraction(30,1));
  caps_capformat->set_value("width", 1280);
  caps_capformat->set_value("height", 720);

  try {
    ret
      ->add(src)
      ->add(conv);

    src
      ->link(conv, caps_capformat);

    ret->add_ghost_pad(conv, "src", "src");

  } catch (const std::runtime_error& ex) {
    cerr << "Unable to initialize dummy webcam source: " << ex.what() << ". " << endl;
  }

  return ret;
}

Glib::RefPtr<Gst::Bin> v4l_webcam(const char* source) {
  Glib::RefPtr<Gst::Bin> ret = Gst::Bin::create("camera");
  Glib::RefPtr<Gst::Caps> caps_capformat;

  Glib::RefPtr<Gst::Element> cam = Gst::ElementFactory::create_element("v4l2src");
  cam->set_property<Glib::ustring>("device", source);

  Glib::RefPtr<Gst::Element> conv = Gst::ElementFactory::create_element("videoconvert");

  // Define filtering capabilities to force video formats
  caps_capformat  = Gst::Caps::create_simple("video/x-raw");
  caps_capformat->set_value("framerate", Gst::Fraction(30,1));
  //caps_capformat->set_value("width", 1920);

  try {
    ret
      ->add(cam)
      ->add(conv);

    cam
      ->link(conv, caps_capformat);

    ret->add_ghost_pad(conv, "src", "src");

  } catch (const std::runtime_error& ex) {
    cerr << "Unable to initialize V4L2 webcam source: " << ex.what() << ". Initiating fallback." << endl;
  }



  return ret;
}


Glib::RefPtr<Gst::Bin> h264_webcam(const char* source) {
  Glib::RefPtr<Gst::Bin> ret = Gst::Bin::create("camera");
  Glib::RefPtr<Gst::Caps> caps_capformat;

  Glib::RefPtr<Gst::Element> cam = Gst::ElementFactory::create_element("uvch264src");
  cam->set_property<Glib::ustring>("device", source);
  cam->set_property<bool>("auto-start", true);

  Glib::RefPtr<Gst::Element> queue = Gst::ElementFactory::create_element("queue");

  // Define filtering capabilities to force video formats
  caps_capformat  = Gst::Caps::create_simple("video/x-h264");
  caps_capformat->set_value("framerate", Gst::Fraction(30,1));
  caps_capformat->set_value("width", 1920);

  Glib::RefPtr<Gst::Element> parse = Gst::ElementFactory::create_element("h264parse");
  Glib::RefPtr<Gst::Element> decode = Gst::ElementFactory::create_element("avdec_h264");
  Glib::RefPtr<Gst::Element> conv = Gst::ElementFactory::create_element("videoconvert");

  try {
    ret
      ->add(cam)
      ->add(queue)
      ->add(parse)
      ->add(decode)
      ->add(conv);

    cam->get_static_pad("vidsrc")->link(queue->get_static_pad("sink"));
    queue
      ->link(parse, caps_capformat)
      ->link(decode)
      ->link(conv);

    ret->add_ghost_pad(conv, "src", "src");

  } catch (const std::runtime_error& ex) {
    cerr << "Unable to initialize H264 webcam source: " << ex.what() << ". Initiating fallback." << endl;
    return v4l_webcam(source);
  }



  return ret;
}


int main(int argc, char** argv)
{

  // Initialize libraries
  Gst::init(argc, argv);
  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.snutt.simhud");

  Glib::RefPtr<Gst::Pipeline> pipeline;
  Glib::RefPtr<Gst::Element> e_source;
  Glib::RefPtr<Gst::Element> e_hud;
  Glib::RefPtr<Gst::Element> e_cvt;
  Glib::RefPtr<Gst::Element> e_sink;

  // Format filters
  Glib::RefPtr<Gst::Caps> caps_informat;
  Glib::RefPtr<Gst::Caps> caps_outformat;

  // Create pipeline:
  pipeline = Gst::Pipeline::create("simhud-pipeline");

  hud = new SimHUD();

  // Create elements:
  if (argc > 1) {
    // Webcam source
    e_source = (Glib::RefPtr<Gst::Element>) v4l_webcam(argv[1]);
  } else {
    // Test source
    e_source = (Glib::RefPtr<Gst::Element>) dummy_webcam();
  }

  e_hud = hud->element();
  e_cvt = Gst::ElementFactory::create_element("videoconvert");

  e_sink = Gst::ElementFactory::create_element("autovideosink");
  e_sink->set_property<bool>("sync", false);


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
    cerr << "Exception while adding: " << ex.what() << endl;
    return 1;
  }

  // Link the elements together:
  try
  {
    e_source
      ->link(e_hud)
      ->link(e_cvt)
      ->link(e_sink);
  }
  catch(const std::runtime_error& error)
  {
    cerr << "Exception while linking: " << error.what() << endl;
  }



  // Create the main loop.
  mainloop = Glib::MainLoop::create();

  // Add a bus watcher and also allow the HUD to listen to bus events
  Glib::RefPtr<Gst::Bus> bus = pipeline->get_bus();
  bus->add_watch(sigc::ptr_fun(&on_bus_message));

  // Let the HUD figure out frame size and stuff
  cout << "Setting to PAUSED." << endl;
  pipeline->set_state(Gst::STATE_PAUSED);

  // Go!
  cout << "Setting to PLAYING." << endl;
  pipeline->set_state(Gst::STATE_PLAYING);

  cout << "Running." << endl;
  //mainloop->run();

  stop = false;
  while (!stop) {
    SensorValues values;
    query_sensors(values);
    hud->set_values(values);

    usleep(500000);
  }

  // Clean up nicely:
  cout << "Returned. Setting state to NULL." << endl;
  pipeline->set_state(Gst::STATE_NULL);



  return 0;
}

