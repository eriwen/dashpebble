#PATH=$PATH:/usr/local/pebble-dev/arm-cs-tools/bin

configure:
	./waf configure

build: configure
	./waf build

clean:
	rm -fr build

serve:
	python -m SimpleHTTPServer &

.PHONY: clean
