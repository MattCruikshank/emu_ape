CC = /opt/cosmo/bin/fatcosmocc
ZIP = /opt/cosmo/bin/zip
CFLAGS =
BINDIR = bin
TARGET = $(BINDIR)/emu_ape
SOURCES = emu_ape.c mongoose.c
EMULATORJS_VERSION = 4.2.3
EMULATORJS_URL = https://github.com/EmulatorJS/EmulatorJS/releases/download/v$(EMULATORJS_VERSION)/$(EMULATORJS_VERSION).7z

all: data/loader.js $(TARGET)
	@echo "Adding assets to executable..."
	$(ZIP) $(TARGET) index.html index.css
	$(ZIP) -r $(TARGET) data/ docs/ images/ roms/

$(TARGET): $(SOURCES) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

$(BINDIR):
	mkdir -p $(BINDIR)

data/loader.js:
	@echo "Downloading EmulatorJS v$(EMULATORJS_VERSION)..."
	curl -L -o /tmp/emulatorjs.7z $(EMULATORJS_URL)
	7z x /tmp/emulatorjs.7z -o. -y
	rm -f /tmp/emulatorjs.7z

ARCADE_CORES = data/cores/fbneo-wasm.data data/cores/fbneo-legacy-wasm.data \
	data/cores/fbneo-thread-wasm.data data/cores/fbneo-thread-legacy-wasm.data \
	data/cores/fbalpha2012_cps1-wasm.data data/cores/fbalpha2012_cps1-legacy-wasm.data \
	data/cores/fbalpha2012_cps1-thread-wasm.data data/cores/fbalpha2012_cps1-thread-legacy-wasm.data \
	data/cores/fbalpha2012_cps2-wasm.data data/cores/fbalpha2012_cps2-legacy-wasm.data \
	data/cores/fbalpha2012_cps2-thread-wasm.data data/cores/fbalpha2012_cps2-thread-legacy-wasm.data

arcade: data/loader.js $(TARGET)
	@echo "Adding assets (arcade only) to executable..."
	$(ZIP) $(TARGET) index.html index.css
	$(ZIP) -r $(TARGET) data/loader.js data/emulator.min.js data/emulator.min.css data/emulator.css data/version.json data/compression/ data/localization/ data/src/ docs/ images/ roms/
	$(ZIP) $(TARGET) $(ARCADE_CORES)

run: all
	./$(TARGET)

run-arcade: arcade
	./$(TARGET)

clean:
	rm -rf $(BINDIR)

clean-all: clean
	rm -rf data/ docs/ CHANGES.md CODE_OF_CONDUCT.md CONTRIBUTING.md LICENSE README.md package.json package-lock.json

.PHONY: all clean clean-all run arcade run-arcade
