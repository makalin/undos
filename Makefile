# undos Makefile
# Builds the LD_PRELOAD library and installs scripts

CC = gcc
CFLAGS = -Wall -Wextra -fPIC -shared -O2
LDFLAGS = -ldl
PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1

# Source files
SOURCES = undos.c
TARGET = undos.so
SCRIPTS = undos-restore

# Default target
all: $(TARGET)

# Build the shared library
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Install the library and scripts
install: $(TARGET)
	@echo "Installing undos..."
	@mkdir -p $(LIBDIR)
	@mkdir -p $(BINDIR)
	@mkdir -p $(MANDIR)
	@cp $(TARGET) $(LIBDIR)/
	@cp $(SCRIPTS) $(BINDIR)/
	@chmod +x $(BINDIR)/$(SCRIPTS)
	@echo "Installation complete!"
	@echo ""
	@echo "Add these lines to your shell profile (~/.bashrc, ~/.zshrc, etc.):"
	@echo "export LD_PRELOAD=$(LIBDIR)/$(TARGET)"
	@echo "export UNDOS_TRASH=~/.local/share/undos_trash"

# Uninstall
uninstall:
	@echo "Uninstalling undos..."
	@rm -f $(LIBDIR)/$(TARGET)
	@rm -f $(BINDIR)/$(SCRIPTS)
	@echo "Uninstallation complete!"

# Clean build artifacts
clean:
	rm -f $(TARGET) *.o *.so

# Test the library
test: $(TARGET)
	@echo "Testing undos library..."
	@mkdir -p test_dir
	@echo "test file" > test_dir/test_file.txt
	@echo "Before deletion:"
	@ls -la test_dir/
	@echo ""
	@echo "Deleting with LD_PRELOAD..."
	@LD_PRELOAD=./$(TARGET) UNDOS_TRASH=./test_trash rm test_dir/test_file.txt
	@echo "After deletion:"
	@ls -la test_dir/ 2>/dev/null || echo "Directory is empty"
	@echo ""
	@echo "Trash contents:"
	@ls -la test_trash/ 2>/dev/null || echo "No trash directory"
	@echo ""
	@echo "Restoring with undos-restore..."
	@./undos-restore test_file.txt
	@echo "After restore:"
	@ls -la test_dir/
	@rm -rf test_dir test_trash

# Show help
help:
	@echo "undos Makefile targets:"
	@echo "  all      - Build the shared library (default)"
	@echo "  install  - Install library and scripts to system"
	@echo "  uninstall- Remove library and scripts from system"
	@echo "  clean    - Remove build artifacts"
	@echo "  test     - Run basic functionality test"
	@echo "  help     - Show this help message"

.PHONY: all install uninstall clean test help
