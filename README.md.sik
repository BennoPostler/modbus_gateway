# Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi

Ein leistungsfähiges Open-Source-Gateway für die Integration von **Modbus RTU**, **Modbus TCP** und **MQTT**.  
Entwickelt für Raspberry Pi (64 Bit, z. B. Raspberry Pi 5), unterstützt lokale Steuerungen, Fernwartung und intelligente Automatisierung im industriellen und privaten Umfeld.

---

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