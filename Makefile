OPENCV_INCLUDE=`pkg-config --cflags opencv`
OPENCV_LIBS=`pkg-config --libs opencv`

all : parallax.cpp
	g++ -c parallax.cpp -o parallax.o -fPIC -O2 -arch i386 -I/usr/include/python2.6 -I/Developer/Panda3D/include ${OPENCV_INCLUDE}
	g++ parallax.o -o parallaxis -fPIC -arch i386 -L/Developer/Panda3D/lib -lp3framework -lpanda -lpandafx -lpandaexpress -lp3dtoolconfig -lp3dtool -lp3pystub -lp3direct ${OPENCV_LIBS}

clean :
	rm parallax.o parallaxis

models :
	find models -name '*.dae' | cut -f 1 -d "." | xargs -If /Developer/Tools/Panda3D/dae2egg f.dae -o f.egg 
	find models -name '*.egg' | cut -f 1 -d "." | xargs -If /Developer/Tools/Panda3D/egg2bam f.egg -o f.bam
.PHONY : models