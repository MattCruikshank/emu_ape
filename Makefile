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
	$(ZIP) -r $(TARGET) data/ docs/

$(TARGET): $(SOURCES) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

$(BINDIR):
	mkdir -p $(BINDIR)

data/loader.js:
	@echo "Downloading EmulatorJS v$(EMULATORJS_VERSION)..."
	curl -L -o /tmp/emulatorjs.7z $(EMULATORJS_URL)
	7z x /tmp/emulatorjs.7z -o. -y
	rm -f /tmp/emulatorjs.7z

run: all
	./$(TARGET)

clean:
	rm -rf $(BINDIR)

clean-all: clean
	rm -rf data/ docs/ CHANGES.md CODE_OF_CONDUCT.md CONTRIBUTING.md LICENSE README.md package.json package-lock.json

.PHONY: all clean clean-all run
