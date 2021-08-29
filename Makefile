all: 	cursary

cursary: 	cursary.cc
	@echo Cursary development version setting up ...
	g++ -lncurses $(CURDIR)/cursary.cc -o cursary
	sudo rm -f /usr/bin/cursary
	sudo cp cursary /usr/bin/cursary
