#include <iostream>
#include <cstring>

#include "pandaFramework.h"
#include "pandaSystem.h"
#include "genericAsyncTask.h"
#include "asyncTaskManager.h"
#include "pointLight.h"
#include "ambientLight.h"
#include "boundingBox.h"
#include "cardMaker.h"

#include "cv.h"
#include "highgui.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700

struct fp {
  double x, y;
  int size;
};

int initialize_camera(void);
struct fp* detect_and_draw(IplImage* image);
struct fp* get_frame_point(void);

const double smoothing_x = 0.65, smoothing_y = 0.6, smoothing_size = 0.8;
const double scale_x = -20., scale_y = 1. / 500, scale_z = -10.;
double x_offset = 0, y_offset = -15000, z_offset = -.2;
const double diff_threshold = 0;

const char* cascade_name =
  "/usr/local/share/opencv/haarcascades/haarcascade_frontalface_alt.xml";

const int input_name = 0;

void print_face(struct fp* face) {
  cout << "<face at x=" << face->x << ", y=" << face->y << ", z=" << face->size << ">" << endl;
}
