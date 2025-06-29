---
title: Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi
layout: default
---

⚙️ Kapitel 4: Installationsanleitung
📦 Voraussetzungen

Bevor das Gateway kompiliert oder genutzt werden kann, müssen folgende Pakete und Tools auf dem Entwicklungsrechner installiert sein:
🔧 Abhängigkeiten unter Fedora / Red Hat

sudo dnf install git cmake gcc-c++ make \
    libmodbus-devel openssl-devel \
    paho-mqtt-c-devel paho-mqtt-cpp-devel

🔧 Abhängigkeiten unter Debian / Raspberry Pi OS

sudo apt install git cmake g++ make \
    libmodbus-dev libssl-dev \
    libpaho-mqtt-dev libpaho-mqttpp-dev

    📌 Alternativ wird ein statisches Build mitgeliefert – siehe Kapitel 15: Deployment.

🧱 Struktur des Projektverzeichnisses

modbus_gateway/
├── include/                  # Header-Dateien (z. B. log.h, config.h)
├── src/                      # Implementierung (.cpp-Dateien)
├── third_party/              # Eingebettete externe Bibliotheken (MQTT)
├── build/                    # Build-Ausgabe (nicht eingecheckt)
├── modbus_gateway.ini        # Konfigurationsdatei
├── Makefile                  # Build-Skript
├── modbus_gateway.service    # systemd-Unit (für Raspberry Pi)
└── modbus_gateway.md         # Diese Dokumentation

🏗️ Kompilieren
Einfacher Build mit dem Makefile:

make

Das erzeugt:

    modbus_gateway: Die ausführbare Datei im Projektverzeichnis

    Optional: statisch gelinkte Variante modbus_gateway_static

    💡 Für ARM (z. B. Raspberry Pi) Cross-Kompilierung siehe Kapitel 6.

🧪 Testlauf ohne Verbindung (--dry-run)

./modbus_gateway modbus_gateway.ini --dry-run

Dadurch wird:

    die INI-Konfiguration vollständig geprüft

    TLS-Zertifikate verifiziert (falls TLS aktiviert)

    keine Verbindung zu MQTT oder Modbus aufgebaut

Ausgabe-Beispiel:

Validierung erfolgreich: Konfiguration wurde geladen.
MQTT-Server: mqtt://192.168.0.82:1883
Modbus-Gerät: /dev/ttyUSB0 @ 19200
TCP-Port: 502

📁 Konfiguration (INI-Datei)

Die Datei modbus_gateway.ini muss entweder:

    als erstes Argument übergeben werden

    standardmäßig unter /etc/modbus_gateway.ini liegen

Beispiel:

./modbus_gateway /etc/modbus_gateway.ini

🧯 Beenden des Gateways

Das Gateway reagiert auf:

    SIGINT (Ctrl+C im Terminal)

    SIGTERM (z. B. durch systemctl stop modbus_gateway)

Dadurch werden:

    alle offenen TCP-Verbindungen geschlossen

    MQTT clean disconnect + LWT-Reset gesendet

    Statistik- und Client-Threads ordentlich beendet

    das Logfile sauber abgeschlossen

🚀 Schnellstart
1. Konfiguration anpassen

cp modbus_gateway.ini.example modbus_gateway.ini
nano modbus_gateway.ini

2. Kompilieren

make

3. Starten

./modbus_gateway modbus_gateway.ini

