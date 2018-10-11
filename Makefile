
all: palette.js

palette.js:
	emcc kmeans.c -O3 -o kmeans.js -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS='["_kmeans", "_malloc", "_free"]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall"]'

clean:
	rm -f kmeans.js kmeans.wasm
