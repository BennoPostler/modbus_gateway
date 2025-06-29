---
title: "Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi"
layout: default
---

🛠 Kapitel 6: Kompilierung & Makefile
🔧 Zielsystem

Das Projekt ist ausgelegt für den Einsatz auf einem Raspberry Pi 5 (aarch64) mit einem modernen Linux-System (z. B. Raspberry Pi OS 64 Bit).
Getestete Umgebungen
Plattform	Status
Raspberry Pi OS 64 bit	✅ stabil
Fedora (Cross-Build)	✅ unterstützt
Debian x86_64 (Build)	✅ mit Cross
🔨 Projektstruktur (relevant für Build)

modbus_gateway/
├── src/                   # C++ Quellcode (.cpp)
├── include/               # Headerdateien (.h)
├── third_party/
│   ├── paho.mqtt.cpp/     # MQTT C++-Bibliothek (mit eingebetteter C-Bibliothek)
│   └── paho.mqtt.c/
├── systemd/               # systemd Service-Dateien
├── tools/                 # Hilfsskripte, Logrotate
├── modbus_gateway.ini     # Konfigurationsdatei (Beispiel)
├── Makefile               # Build-System
└── README.md              # Dokumentation

📦 Abhängigkeiten

Die wichtigsten Abhängigkeiten sind lokal eingebettet:
Bibliothek	Quelle	Status
libmodbus	Systembibliothek	⬅ erforderlich
Paho MQTT C++	third_party/paho.mqtt.cpp	✅ integriert
Paho MQTT C	third_party/paho.mqtt.c	✅ integriert
Paketinstallation (Build-Host)
Fedora:

sudo dnf install libmodbus-devel gcc-c++ make openssl-devel

Debian/Raspberry Pi OS:

sudo apt install libmodbus-dev build-essential libssl-dev

🧪 Cross-Compile für Raspberry Pi 5

Optional kann auf einem x86_64-System für ARM64 (aarch64) kompiliert werden.
Toolchain-Voraussetzung

sudo dnf install gcc-aarch64-linux-gnu

Beispielkonfiguration siehe Datei: toolchain-aarch64.cmake
🧰 Makefile – Übersicht

Das Projekt wird über ein zentrales Makefile verwaltet.

make             # Standard-Build für native Plattform
make clean       # Bereinigt alle erzeugten Dateien
make install     # Installation ins Zielsystem (z. B. /usr/local/bin)
make tarball     # Erzeugt ein .tar.gz Release-Paket

Für Cross-Compilation:

make CROSS=1     # Build für Raspberry Pi (aarch64)

📂 Ausgaben
Ziel	Beschreibung
bin/modbus_gateway	Finales Binary
build/	Objektdateien + Abhängigkeiten
modbus_gateway.tar.gz	Optionales Release-Archiv
🔍 Beispiel: Kompilieren auf dem Pi

git clone https://github.com/deinuser/modbus_gateway.git
cd modbus_gateway
make

🔍 Beispiel: Kompilieren auf x86 für Raspberry Pi

make CROSS=1

→ Vorausgesetzt: Cross-Toolchain ist in $PATH, z. B. aarch64-linux-gnu-g++.

