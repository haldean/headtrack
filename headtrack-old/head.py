#!/usr/bin/env python

import collections
import Image
import ImageChops
import math
import numpy
import opencv as cv

from opencv import highgui
from scipy import ndimage

class HeadTrack(object):
    def __init__(self):
        """Initialize the camera and the pygame subsystem."""
        # Initialize the camera and set the dimensions. I use a small
        # frame size because speed is more important than accuracy.
        self.camera = cv.highgui.cvCreateCameraCapture(0)
        highgui.cvSetCaptureProperty(self.camera, cv.highgui.CV_CAP_PROP_FRAME_WIDTH, 160)
        highgui.cvSetCaptureProperty(self.camera, cv.highgui.CV_CAP_PROP_FRAME_HEIGHT, 120)

        self.blur_factor = 0
        self.diff_threshold = 40
        self.update_threshold = 1500
        self.mean_period = 6
        self.last_image = None
        self.last_centers = collections.deque()
        self.scale_mean = 0.3

        self.last_mean = None
        self.this_mean = None
        self.diff_mean = None

    def get_image(self):
        """Return an image in PIL (Python Imaging Library) format."""
        im = None
        while not im:
            im = highgui.cvQueryFrame(self.camera)

        arr = numpy.asarray(cv.adaptors.Ipl2PIL(im)).copy()
        arr[:,:,2] = 0

        if self.blur_factor:
            arr = ndimage.gaussian_filter(arr, self.blur_factor)

        pil = Image.fromarray(arr).convert('L')
        return pil, pil.mode

    def get_filtered_image(self, show_line=True):
        """Get an image filtered to display on the screen, and process
        that image to find the number of fingers present."""

        im, mode = self.get_image()

        if self.last_image != None:
            self.last_image, im = im, ImageChops.difference(im, self.last_image)
        else:
            self.last_image = im
        
        def threshold(x):
            if (x > self.diff_threshold):
                return 255
            else: return 0

        thresh = im.point(threshold)
        thresh_arr = numpy.asarray(thresh)

        if thresh.histogram()[-1] > self.update_threshold:
            center = ndimage.center_of_mass(thresh_arr)
            self.last_centers.append(center)
            if len(self.last_centers) > self.mean_period:
                self.last_centers.popleft()

        mean_center = (
            sum([x[0] for x in self.last_centers]) / self.mean_period,
            sum([x[1] for x in self.last_centers]) / self.mean_period)

        mean_center = (mean_center[1] / im.size[0] - .5, 
                       mean_center[0] / im.size[1] - .5)

        mean_center = (mean_center[0] * self.scale_mean, 
                       mean_center[0] * self.scale_mean)

        print mean_center
        return mean_center

    def start(self):
        running = True

        # Main loop
        while running:
            im, cm = self.get_filtered_image()
            if not math.isnan(cm[0]) and not math.isnan(cm[1]):
                last_mean, this_mean = this_mean, cm
                if last_mean:
                    diff_mean = (this_mean[0] - last_mean[0], this_mean[1] - last_mean[1])
            print diff_mean

if __name__ == '__main__':
    head = HeadTrack()
    head.start()
