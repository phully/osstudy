all: ImageMaker.exe

ImageMaker.exe: ImageMaker.c
	gcc -o $@ $<
	cp $@ ../../$@
clean:
	rm -f ImageMaker.exe
	rm -f ../../ImageMaker.exe