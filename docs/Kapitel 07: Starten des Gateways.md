---
title: Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi
layout: default
---

🚀 Kapitel 7: Starten des Gateways
🖥️ Voraussetzungen

Stelle sicher, dass folgende Komponenten auf deinem Raspberry Pi oder Zielsystem vorhanden sind:

    Modbus RTU Gerät via /dev/ttyAMA0, /dev/ttyUSB0, etc.

    MQTT-Broker (lokal oder extern, z. B. Mosquitto)

    Konfigurationsdatei modbus_gateway.ini mit gültigen Parametern

🔧 Start über Terminal

./modbus_gateway /etc/modbus_gateway.ini

Optional:

./modbus_gateway /etc/modbus_gateway.ini --dry-run

📋 Aufrufparameter
Parameter	Beschreibung
[config.ini]	Pfad zur Konfigurationsdatei (Standard: /etc/modbus_gateway.ini)
--dry-run	Validiert nur die Konfiguration, startet keine Netzwerkverbindungen
--validate-only	Synonym für --dry-run
--version	Gibt die aktuelle Versionsnummer aus
--help / -h	Zeigt Hilfetext und verfügbare Optionen an
🧪 Beispielaufrufe

Standardbetrieb mit Konfig-Datei:

./modbus_gateway /etc/modbus_gateway.ini

Nur Konfiguration testen:

./modbus_gateway /etc/modbus_gateway.ini --dry-run

Versionsinfo anzeigen:

./modbus_gateway --version

🏁 Startmeldung (Beispielausgabe)

Starte Gateway...
→ TLS ist aktiviert → MQTT-Verbindung erfolgt verschlüsselt.
→ MQTT-Broker: mqtts://192.168.0.82
→ Modbus RTU: /dev/ttyAMA0 @ 19200 baud
→ TCP-Server aktiv auf Port 502

🧼 Beenden des Gateways

Beende das Programm mit CTRL+C oder sende ein SIGINT/SIGTERM Signal. Der Shutdown erfolgt sauber:

    MQTT-Statusmeldung wird als LWT gesendet

    Modbus & Sockets werden geschlossen

    Alle Threads (Clients, Statistik) werden synchronisiert

🧩 Nächste Schritte

→ Wenn du systemweite Integration planst (z. B. Autostart beim Boot), lies weiter in:

➡️ Kapitel 14: systemd-Integration

