# Postmelodic

JACK+OSC sound player thingy with cuepoints and memory leaks.

## Command line

* `-n` set JACK client name
* `-c` try to autoconnect to JACK port
* `-p` set OSC control port
* `-v` verbose output

## OSC receive

* `/load` `int str`
* `/play` `int int`
* `/cue` `int int int`
* `/stop` `int`
* `/listen` `str str`
* `/listen` `str`
* `/listen`

## OSC send

* `/loaded` `str int str int int int` 
* `/playing` `str int int int`
* `/stopped` `str int`
