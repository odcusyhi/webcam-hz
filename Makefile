webcam-hz: webcam-hz.c
	clang -O2 -Wall -o webcam-hz webcam-hz.c -framework IOKit -framework CoreFoundation

clean:
	rm -f webcam-hz
