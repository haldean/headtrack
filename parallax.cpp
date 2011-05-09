#include "parallax.h"

PandaFramework framework;

static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;
static CvCapture* capture = 0;
struct fp face;
struct fp smoothed;

LVector3f last_loc = NULL;

PT(AsyncTaskManager) taskManager = AsyncTaskManager::get_global_ptr();
NodePath model, camera;

AsyncTask::DoneStatus updateLocationTask(GenericAsyncTask* task, void* data) {
  struct fp* face = get_frame_point();

  LVector3f loc_vector = 
    LVector3f(scale_x * (x_offset + face->x),
	      scale_y * (y_offset + face->size),
	      scale_z * (z_offset + face->y));

  if (last_loc != NULL) {
    LVector3f diff = loc_vector - last_loc;
    if (diff.length() > diff_threshold) {
      camera.set_pos(loc_vector);
    }
  } else {
    camera.set_pos(loc_vector);
  }

  last_loc = loc_vector;

  // Tell the task manager to continue this task the next frame.
  return AsyncTask::DS_cont;
}
 
int main(int argc, char *argv[]) {
  /* Initialize facedetect. */
  int status = initialize_camera();
  if (status == -1) {
    cerr << "Unable to initialize camera." << endl;
    return -1;
  }

  framework.open_framework(argc, argv);

  WindowProperties properties = WindowProperties::get_default();
  properties.set_size(WINDOW_WIDTH, WINDOW_HEIGHT);
  properties.set_title("Parallax");

  WindowFramework *window = framework.open_window(properties, 0, NULL, NULL);
  window->set_background_type(WindowFramework::BT_white);

  camera = window->get_camera_group();
  camera.set_pos_hpr(0, -20, 0, 0, 0, 0);
 
  PT(PointLight) point_light = new PointLight("Point Light");
  point_light->set_color(LVecBase4f(.9, .9, .8, 1.));

  NodePath light_node = window->get_render().attach_new_node(point_light);
  light_node.set_pos(5, -20, 10);
  window->get_render().set_light(light_node);

  PT(AmbientLight) ambient = new AmbientLight("Ambient Light");
  ambient->set_color(LVecBase4f(.3, .3, .3, 1.));
  NodePath ambient_node = window->get_render().attach_new_node(ambient);
  window->get_render().set_light(ambient_node);

  string model_path = "./models/" + string(argv[1]);
  model = window->load_model(framework.get_models(), model_path);
  model.reparent_to(window->get_render());
 
  taskManager->add(new GenericAsyncTask("Update the location of the cube", 
					&updateLocationTask, (void*) NULL));

  //do the main loop, equal to run() in python
  framework.main_loop();
  //close the window framework
  framework.close_framework();
  return (0);
}


int run(void) {
  int status = initialize_camera();
  if (status == -1) {
    return -1;
  }

  struct fp* face;
  for (;;) {
    face = get_frame_point();
    cout << face->x << ", " << face->y
      << ", " << face->size << endl;
  }
  return 0;
}

int initialize_camera(void) {
  // Load the HaarClassifierCascade
  cascade = (CvHaarClassifierCascade*) cvLoad(cascade_name, 0, 0, 0);
    
  // Check whether the cascade has loaded successfully. Else report and error and quit
  if(!cascade) {
    cerr << "Could not load classifier cascade" << endl;
    return -1;
  }
    
  // Allocate the memory storage
  storage = cvCreateMemStorage(0);
  capture = cvCaptureFromCAM(input_name);

  smoothed.x = 0;
  smoothed.y = 0;
  smoothed.size = 0;

  // Create a new named window with title: result
  cvNamedWindow("FaceDetection", 1);
  return 0;
}

struct fp* get_frame_point(void) {
  // Images to capture the frame from video or camera or from file
  IplImage *frame, *frame_copy = 0;

  if(!cvGrabFrame(capture)) {
    cerr << "Grab frame failed." << endl;
    return NULL;
  }

  frame = cvRetrieveFrame(capture);

  // If the frame does not exist, quit the loop
  if(!frame) {
    cerr << "Frame could not be retrieved." << endl;
    return NULL;
  }
  
  // Allocate framecopy as the same size of the frame
  if(!frame_copy)
    frame_copy = cvCreateImage(cvSize(frame->width,frame->height),
			       IPL_DEPTH_8U, frame->nChannels);

  // Check the origin of image. If top left, copy the image frame to frame_copy. 
  if(frame->origin == IPL_ORIGIN_TL)
    cvCopy(frame, frame_copy, 0);
  else
    cvFlip(frame, frame_copy, 0);

  // Call the function to detect and draw the face
  return detect_and_draw(frame_copy);
}

struct fp* detect_and_draw( IplImage* img ) {
  int scale = 5;

  // Create a new image based on the input image
  IplImage* gray = cvCreateImage(cvSize(img->width, img->height), 8, 1);
  IplImage* small = cvCreateImage(cvSize(img->width/scale, img->height/scale), 8, 1);

  cvCvtColor(img, gray, CV_BGR2GRAY);
  cvResize(gray, small, CV_INTER_LINEAR);
  cvEqualizeHist(small, small);

  int i;

  // Clear the memory storage which was used before
  cvClearMemStorage(storage);

  // Find whether the cascade is loaded, to find the faces. If yes, then:
  if(cascade) {
    // Detect the objects and store them in the sequence
    CvSeq* faces = cvHaarDetectObjects(small, cascade, storage, 1.1, 2, 
				       CV_HAAR_DO_CANNY_PRUNING, cvSize(20, 20));

    // Loop the number of faces found.
    for( i = 0; i < (faces ? faces->total : 0); i++ ) {
      // Create a new rectangle for drawing the face
      CvRect* r = (CvRect*)cvGetSeqElem( faces, i );

      int center_x = (r->x + 0.5 * r->width) * scale;
      int center_y = (r->y + 0.5 * r->height) * scale;

      cvCircle(img, cvPoint(center_x, center_y), 5, CV_RGB(255,0,0), -1, 8, 0);

      face.x = ((double) center_x / (double) img->width) - .5;
      face.y = ((double) center_y / (double) img->height) - .5;
      face.size = r->width * r->height;

      smoothed.x = (1 - smoothing_x) * face.x + smoothing_x * smoothed.x;
      smoothed.y = (1 - smoothing_y) * face.y + smoothing_y * smoothed.y;
      smoothed.size = (1 - smoothing_size) * face.size + smoothing_size * smoothed.size;
    }
  }

  cvShowImage("FaceDetection", img);

  // Release the temp images created.
  cvReleaseImage(&small);
  cvReleaseImage(&gray);

  return &smoothed;
}

