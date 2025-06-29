# ---------------------------
# Projektkonfiguration
# ---------------------------
CROSS_COMPILE := aarch64-linux-gnu
SYSROOT := $(HOME)/sysroot-aarch64

VERSION := 1.0.1
RELEASE_NAME := modbus_gateway-$(VERSION)
RELEASE_DIR := /tmp/$(RELEASE_NAME)
RELEASE_FILE := $(RELEASE_NAME).tar.gz


SRC_DIR := src
BUILD_DIR := build
TARGET_DIR := target
TARGET := $(TARGET_DIR)/modbus_gateway

PAHO_CPP_DIR := third_party/paho.mqtt.cpp
PAHO_C_DIR := third_party/paho.mqtt.c

PAHO_CPP_LIB := $(PAHO_CPP_DIR)/build/src/libpaho-mqttpp3.a
PAHO_C_LIB := $(PAHO_C_DIR)/build/src/libpaho-mqtt3as.a

# ---------------------------
# Compiler und Linker
# ---------------------------
CXX := $(CROSS_COMPILE)-g++

CXXFLAGS := -Wall -g -std=c++17 --sysroot=$(SYSROOT) \
  -Iinclude \
  -I$(PAHO_CPP_DIR)/src \
  -I$(PAHO_C_DIR)/src \
  -I$(SYSROOT)/usr/include \
  -I$(SYSROOT)/usr/include/c++/12 \
  -I$(SYSROOT)/usr/include/c++/12/aarch64-linux-gnu \
  -I$(SYSROOT)/usr/lib/gcc/aarch64-linux-gnu/12/include \
  -I$(SYSROOT)/usr/include/aarch64-linux-gnu/c++/12 \
  -I$(SYSROOT)/usr/include/aarch64-linux-gnu

LDFLAGS := 	--sysroot=$(SYSROOT) \
			-L$(SYSROOT)/usr/lib \
			-L$(SYSROOT)/lib \
			-L$(SYSROOT)/usr/lib/aarch64-linux-gnu \
			-L$(SYSROOT)/usr/lib/gcc/aarch64-linux-gnu/12 \
			-lpthread \
			-lssl \
			-lcrypto \
			-lmodbus \
			-lstdc++ \
			-lgcc_s \
			-lgcc

STATIC_LIBS := $(PAHO_CPP_LIB) $(PAHO_C_LIB)

# ---------------------------
# Quellen und Objekte
# ---------------------------
SRCS := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.c)

OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(filter %.cpp, $(SRCS))) \
        $(patsubst $(SRC_DIR)/%.c,   $(BUILD_DIR)/%.o, $(filter %.c,   $(SRCS)))

# ---------------------------
# Regeln
# ---------------------------
all: check_sysroot $(TARGET)

# Ziel-Binary
$(TARGET): $(OBJS) | $(TARGET_DIR)
	$(CXX) -o $@ $(OBJS) $(STATIC_LIBS) $(LDFLAGS)

# Objektdateien erzeugen
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Verzeichnisse
$(BUILD_DIR):
	mkdir -p $@

$(TARGET_DIR):
	mkdir -p $@

# ---------------------------
# Sysroot-Check
# ---------------------------
check_sysroot:
	@if ! find $(SYSROOT)/usr/include/c++ -name iostream -print -quit | grep -q iostream; then \
		echo "❌ Fehler: <iostream> fehlt im Sysroot ($(SYSROOT))."; \
		echo "👉 Bitte sysroot vollständig kopieren oder libstdc++ installieren."; \
		exit 1; \
	else \
		echo "✅ iostream im Sysroot gefunden."; \
	fi


release:
	@echo "Erzeuge Release-Verzeichnis..."
	rm -rf $(RELEASE_DIR)
	mkdir -p $(RELEASE_DIR)

	@echo "Kopiere Dateien..."
	cp -r src include third_party docs $(RELEASE_DIR)
	cp src/modbus_gateway.cpp modbus_gateway.ini Makefile modbus_gateway.service README.md LICENSE $(RELEASE_DIR)

	@echo "Erzeuge Archiv..."
	tar -czf releases/$(RELEASE_FILE) -C /tmp $(RELEASE_NAME)
	@echo "Release-Datei erstellt: releases/$(RELEASE_FILE)"


# ---------------------------
# Aufräumen
# ---------------------------
clean:
	rm -rf $(BUILD_DIR) $(TARGET_DIR)

.PHONY: all clean check_sysroot
