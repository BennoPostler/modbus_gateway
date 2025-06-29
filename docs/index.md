---
title: "Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi"
layout: default
---


# Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi

Ein leistungsfähiges Open-Source-Gateway für die Integration von **Modbus RTU**, **Modbus TCP** und **MQTT**.  
Entwickelt für Raspberry Pi (64 Bit, z. B. Raspberry Pi 5), unterstützt lokale Steuerungen, Fernwartung und intelligente Automatisierung im industriellen und privaten Umfeld.

---
# Index
[Kapitel-01-Einleitung](Kapitel-01-Einleitung.md)
[Kapitel-02-Systemueberblick](Kapitel-02-Systemueberblick.md)
[Kapitel-03-Architektur](Kapitel-03-Architektur.md)
[Kapitel-04-Installationsanleitung](Kapitel-04-Installationsanleitung.md)
[Kapitel-05-Konfigurationsdatei-modbus_gateway.ini](Kapitel-05-Konfigurationsdatei-modbus_gateway.ini.md)
[Kapitel-06-Kompilierung-und-Makefile](Kapitel-06-Kompilierung-und-Makefile.md)
[Kapitel-07-Starten-des-Gateways](Kapitel-07-Starten-des-Gateways.md)
[Kapitel-08-MQTT-Integration](Kapitel-08-MQTT-Integration.md)
[Kapitel-09-Modbus-TCP↔RTU-Kommunikation](Kapitel-09-Modbus-TCP↔RTU-Kommunikation.md)
[Kapitel-10-Diagnose-und-Statistik](Kapitel-10-Diagnose-und-Statistik.md)
[Kapitel-11-TLS-und-Sicherheit](Kapitel-11-TLS-und-Sicherheit.md)
[Kapitel-12-Logging](Kapitel-12-Logging.md)
[Kapitel-13-Fehlerbehandlung](Kapitel-13-Fehlerbehandlung.md)
[Kapitel-14-Systemd-Integration](Kapitel-14-Systemd-Integration.md)
[Kapitel-15-Deployment](Kapitel-15-Deployment.md)
[Kapitel-16-Beispiele–Modbus-TCP-MQTT-YAML-Konfiguration](Kapitel-16-Beispiele–Modbus-TCP-MQTT-YAML-Konfiguration.md)
[Kapitel-17-Troubleshooting-und-Fehlersuche](Kapitel-17-Troubleshooting-und-Fehlersuche.md)
[Kapitel-18-Erweiterungsmöglichkeiten-und-Roadmap](Kapitel-18-Erweiterungsmöglichkeiten-und-Roadmap.md)
[Kapitel-19-Codeuebersicht-und-Projektstruktur](Kapitel-19-Codeuebersicht-und-Projektstruktur.md)
[Kapitel-20-Quellcode-Erklaerung–modbus_gateway.cpp](Kapitel-20-Quellcode-Erklaerung–modbus_gateway.cpp.md)
[Kapitel-21-Verwendete-Bibliotheken-und-Abhaengigkeiten](Kapitel-21-Verwendete-Bibliotheken-und-Abhaengigkeiten.md)
[Kapitel-22-Lizenz](Kapitel-22-Lizenz.md)
[Kapitel-23-Anhang](Kapitel-23-Anhang.md)


## 🔧 Hauptfunktionen

- 📡 **Modbus TCP ↔ RTU Weiterleitung** (Protokollkonvertierung)  
- 🔌 **MQTT-Integration** für Modbus-Register (Read/Write über Topics)  
- 🔍 **Diagnosefunktionen nach Modbus-Norm (FC 0x08)**  
- 📊 **Statistiken & Echtzeit-Monitoring** via MQTT  
- 🔐 **TLS-Unterstützung für sichere MQTT-Verbindungen**  
- 🧪 **Dry-Run-Modus** zur Konfigurationsprüfung ohne Verbindungsaufbau  
- 🧰 **Systemd-Service & Logging** für produktiven Dauerbetrieb  

---

## 📁 Projektstruktur

```text
modbus_gateway/
├── src/                  → Hauptlogik (modbus_gateway.cpp)
├── include/              → Header-Dateien
├── third_party/          → Eingebettete Bibliotheken (Paho MQTT, libmodbus)
├── docs/                 → Vollständige Dokumentation
├── modbus_gateway.ini    → Konfigurationsdatei
├── Makefile              → Build-Skript
└── modbus_gateway.service → systemd-Serviceeinheit

🧪 Beispiel-MQTT-Topics

Holding Register lesen:

modbus/read/holding/1/40001

Coil schreiben:

Topic: modbus/write/coils/3/8
Payload: true

Statistik abrufen:

modbus/command/stats
→ Antwort auf: modbus/stats

📦 Installation (Kurzfassung)

git clone https://github.com/BennoPostler/modbus_gateway.git
cd modbus_gateway
make
sudo make install

    🔧 Für Details siehe docs/INSTALL.md

🛠 Abhängigkeiten

    libmodbus – Modbus RTU & TCP

    Paho MQTT C/C++ – MQTT-Client

    Raspberry Pi OS 64-bit + g++ (Cross-Compile oder lokal)

📄 Lizenz

MIT License – frei nutzbar und anpassbar für eigene Projekte.
📚 Dokumentation

→ Die vollständige Dokumentation findest du unter docs/modbus_gateway.md oder bald als GitHub Pages unter:
🔗 https://bennopostler.github.io/modbus_gateway
🤝 Beiträge willkommen