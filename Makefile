CC = /opt/cosmo/bin/fatcosmocc
ZIP = /opt/cosmo/bin/zip
CFLAGS =
BINDIR = bin
TARGET = $(BINDIR)/emu_ape
SOURCES = emu_ape.c mongoose.c

all: $(TARGET)
	@echo "Adding assets to executable..."
	$(ZIP) $(TARGET) index.html index.css
	$(ZIP) -r $(TARGET) data/ docs/

$(TARGET): $(SOURCES) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

$(BINDIR):
	mkdir -p $(BINDIR)

run: all
	./$(TARGET)

clean:
	rm -rf $(BINDIR)

.PHONY: all clean run
