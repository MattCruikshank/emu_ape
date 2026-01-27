CC = /opt/cosmo/bin/fatcosmocc
ZIP = /opt/cosmo/bin/zip
CFLAGS =
BINDIR = bin
TARGET = $(BINDIR)/server
SOURCES = hello.c mongoose.c
ASSETS = index.html index.css

all: $(TARGET)
	@echo "Adding assets to executable..."
	$(ZIP) $(TARGET) $(ASSETS)

$(TARGET): $(SOURCES) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

$(BINDIR):
	mkdir -p $(BINDIR)

run: all
	./$(TARGET)

clean:
	rm -rf $(BINDIR)

.PHONY: all clean run
