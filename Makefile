ALL:setup.bin kernel.bin

setup.bin:
	cd setup; make clean; make;

kernel.bin:
	cd kernel; make clean; make;

