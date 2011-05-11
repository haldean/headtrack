#!/usr/bin/env python

import os, soya, sys

soya.path.append(os.path.join(os.path.dirname(sys.argv[0]), "data"))
soya.init(title='Head Tracking')

class TrackModel(soya.Body):
    def __init__(self, scene, model):
        soya.Body.__init__(self, scene, model)
        self.move = None
        
    def set_headtracker(self, track):
        self.track = track

    def set_move(self, dx, dy, dz):
        self.move = soya.Vector(self, dx, dy, dz)

    def advance_time(self, proportion):
        soya.Body.advance_time(self, proportion)

        if self.move:
            self.add_mul_vector(proportion, self.move)

scene = soya.World()

model = soya.Model.get('cube')
body = TrackModel(scene, model)

light = soya.Light(scene)
light.set_xyz(2., 0., 2.)

camera = soya.Camera(scene)
camera.set_xyz(0, 0, 10)

soya.set_root_widget(camera)
soya.MainLoop(scene).main_loop()
