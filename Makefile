all: 	cursary

cursary: 	cursary.cc
	@echo COMPILING SOURCE FILES
	g++ -lncurses $(CURDIR)/cursary.cc -o cursary
	@echo REMOVING OLD BINARY
	sudo rm -f /usr/bin/cursary
	@echo MOVING NEW BINARY
	sudo cp cursary /usr/bin/cursary
