#!/usr/bin/env python

import collections
import Image
import ImageChops
import math
import numpy
import opencv as cv
import pygame

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

        # Initialize pygame and create the font to use for painting
        # numbers to the screen.
        pygame.init()

        self.blur_factor = 0
        self.diff_threshold = 40
        self.update_threshold = 1500
        self.mean_period = 6
        self.last_image = None
        self.last_centers = collections.deque()
        self.scale_mean = 0.3

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

        return (thresh.convert('RGB'), mean_center)

    def start(self):
        # Initialize the feedback window.
        window = pygame.display.set_mode((1600,1000))
        screen = pygame.display.get_surface()
        running = True

        w, h = screen.get_width(), screen.get_height()

        # Main loop
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    print "Exiting"
                    running = False

            im, cm = self.get_filtered_image()

            screen.fill((0,0,0))
            pg_img = pygame.image.frombuffer(im.tostring(), im.size, im.mode)
            screen.blit(pg_img, (0,0))
            if not math.isnan(cm[0]) and not math.isnan(cm[1]):
                pos_back = (int(w * (.5 - cm[0])), int(h / 2))
                pygame.draw.circle(screen, (0,255,0), pos_back, 100)

                pos_front = (int(w * (.5 + cm[0])), int(h / 2))
                pygame.draw.circle(screen, (255,0,0), pos_front, 150)
            

            # Swap the frame buffer with the displayed frame.
            pygame.display.flip()


if __name__ == '__main__':
    head = HeadTrack()
    head.start()
