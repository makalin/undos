#!/bin/bash

# undos Demo Script
# This script demonstrates the functionality of undos

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

# Demo directory
DEMO_DIR="./undos_demo"
TRASH_DIR="./demo_trash"

echo -e "${BLUE}🗑️  undos Demo${NC}"
echo "=============="
echo ""

# Clean up any previous demo
cleanup() {
    echo -e "${YELLOW}Cleaning up demo...${NC}"
    rm -rf "$DEMO_DIR" "$TRASH_DIR"
    echo -e "${GREEN}✓ Cleanup complete${NC}"
}

# Set up demo environment
setup_demo() {
    echo -e "${BLUE}Setting up demo environment...${NC}"
    
    # Create demo directory
    mkdir -p "$DEMO_DIR"
    
    # Create some test files
    echo "This is a test file" > "$DEMO_DIR/test_file.txt"
    echo "Another test file" > "$DEMO_DIR/another_file.txt"
    echo "Config file" > "$DEMO_DIR/config.conf"
    
    # Create a subdirectory with files
    mkdir -p "$DEMO_DIR/subdir"
    echo "File in subdirectory" > "$DEMO_DIR/subdir/subfile.txt"
    echo "Another subfile" > "$DEMO_DIR/subdir/another_subfile.txt"
    
    # Create a nested directory structure
    mkdir -p "$DEMO_DIR/nested/deep/structure"
    echo "Deep file" > "$DEMO_DIR/nested/deep/structure/deep_file.txt"
    
    echo -e "${GREEN}✓ Demo environment ready${NC}"
    echo ""
}

# Show current state
show_state() {
    echo -e "${PURPLE}Current state:${NC}"
    echo "=============="
    if [[ -d "$DEMO_DIR" ]]; then
        echo -e "${GREEN}Demo directory contents:${NC}"
        find "$DEMO_DIR" -type f | sort
    else
        echo -e "${RED}Demo directory not found${NC}"
    fi
    echo ""
}

# Test file deletion
test_file_deletion() {
    echo -e "${BLUE}Testing file deletion...${NC}"
    
    # Set up environment for undos
    export LD_PRELOAD="./undos.so"
    export UNDOS_TRASH="$TRASH_DIR"
    
    # Delete a single file
    echo -e "${YELLOW}Deleting test_file.txt...${NC}"
    LD_PRELOAD="./undos.so" UNDOS_TRASH="$TRASH_DIR" rm "$DEMO_DIR/test_file.txt"
    
    # Delete another file
    echo -e "${YELLOW}Deleting another_file.txt...${NC}"
    LD_PRELOAD="./undos.so" UNDOS_TRASH="$TRASH_DIR" rm "$DEMO_DIR/another_file.txt"
    
    echo -e "${GREEN}✓ Files deleted (moved to trash)${NC}"
    echo ""
}

# Test directory deletion
test_directory_deletion() {
    echo -e "${BLUE}Testing directory deletion...${NC}"
    
    # Delete a directory
    echo -e "${YELLOW}Deleting subdir...${NC}"
    LD_PRELOAD="./undos.so" UNDOS_TRASH="$TRASH_DIR" rm -rf "$DEMO_DIR/subdir"
    
    echo -e "${GREEN}✓ Directory deleted (moved to trash)${NC}"
    echo ""
}

# Show trash contents
show_trash() {
    echo -e "${PURPLE}Trash contents:${NC}"
    echo "==============="
    if [[ -d "$TRASH_DIR" ]]; then
        find "$TRASH_DIR" -type f | sort
    else
        echo "No trash directory found"
    fi
    echo ""
}

# Test restoration
test_restoration() {
    echo -e "${BLUE}Testing file restoration...${NC}"
    
    # List deleted files
    echo -e "${YELLOW}Listing deleted files:${NC}"
    UNDOS_TRASH="$TRASH_DIR" ./undos-restore --list
    echo ""
    
    # Restore a file
    echo -e "${YELLOW}Restoring test_file.txt...${NC}"
    UNDOS_TRASH="$TRASH_DIR" ./undos-restore test_file.txt
    echo ""
    
    # Show state after restoration
    show_state
}

# Test interactive restoration
test_interactive_restore() {
    echo -e "${BLUE}Testing interactive restoration...${NC}"
    
    if command -v fzf >/dev/null 2>&1; then
        echo -e "${YELLOW}Using fzf for interactive restore...${NC}"
        UNDOS_TRASH="$TRASH_DIR" ./undos-restore --interactive
    else
        echo -e "${YELLOW}fzf not available, showing list instead...${NC}"
        UNDOS_TRASH="$TRASH_DIR" ./undos-restore --list
    fi
    echo ""
}

# Main demo function
run_demo() {
    echo -e "${BLUE}Starting undos demonstration...${NC}"
    echo ""
    
    # Check if undos.so exists
    if [[ ! -f "./undos.so" ]]; then
        echo -e "${RED}Error: undos.so not found. Please run 'make' first.${NC}"
        exit 1
    fi
    
    # Check if undos-restore script exists
    if [[ ! -f "./undos-restore" ]]; then
        echo -e "${RED}Error: undos-restore script not found.${NC}"
        exit 1
    fi
    
    setup_demo
    show_state
    
    echo -e "${BLUE}Now we'll demonstrate undos functionality...${NC}"
    echo ""
    
    test_file_deletion
    show_state
    show_trash
    
    test_directory_deletion
    show_state
    show_trash
    
    test_restoration
    show_trash
    
    echo -e "${BLUE}Demo complete!${NC}"
    echo ""
    echo -e "${YELLOW}To clean up, run: $0 cleanup${NC}"
}

# Handle command line arguments
case "${1:-demo}" in
    demo)
        run_demo
        ;;
    cleanup)
        cleanup
        ;;
    *)
        echo "Usage: $0 [demo|cleanup]"
        echo "  demo     - Run the demonstration (default)"
        echo "  cleanup  - Clean up demo files"
        exit 1
        ;;
esac
