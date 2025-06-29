---
title: "Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi"
layout: default
---

🪵 Kapitel 12: Logging
📋 Ziel des Loggings

Das Gateway führt ein internes Logbuch, um alle relevanten Ereignisse, Fehler und Statusmeldungen systematisch zu protokollieren. Dies erleichtert:

    Debugging und Fehlersuche

    Analyse von Verbindungsproblemen

    Nachvollziehbarkeit von Modbus-/MQTT-Abläufen

🧩 Implementierung
🔧 Zentrale Logfunktion

Die Datei modbus_gateway.cpp enthält eine zentrale log()-Funktion:

void log(const std::string &msg)
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%F %T") << " - " << msg;

    std::string fullMsg = ss.str();
    std::cout << fullMsg << std::endl;
    if (logFile.is_open())
        logFile << fullMsg << std::endl;
}

⏺ Format der Ausgabe

YYYY-MM-DD HH:MM:SS - <Nachricht>

Beispiel:

2025-06-26 19:45:12 - Modbus-TCP-Anfrage empfangen, Länge: 12

🧾 Logdatei

Die Logdatei wird automatisch beim Start des Gateways geöffnet:

logFile.open(config.logFilePath, std::ios::app);

🗂 Konfigurierbarer Pfad

logFilePath = /var/log/modbus_gateway.log

Wird diese Datei nicht explizit gesetzt, schreibt das Gateway nur in die Konsole.
🧮 Log-Level

Das aktuelle Logging ist informativ und unstrukturiert. Es gibt keine formellen Log-Level wie DEBUG / INFO / WARN / ERROR, aber es könnten folgende Strategien manuell übernommen werden:
Nachrichtentyp	Beispiel
🔹 Info	„Starte Gateway...“
⚠️ Warnung	„Kein CA-Zertifikat angegeben“
❌ Fehler	„Fehler bei MQTT-Verbindung: ...“
✅ Erfolg	„MQTT erfolgreich verbunden.“

Eine Erweiterung um formelle Log-Level (z. B. per Enum) wäre optional sinnvoll.
🧪 Beispielhafte Logzeilen

2025-06-26 18:17:43 - Starte Gateway...
2025-06-26 18:17:43 - TLS ist aktiviert → MQTT-Verbindung erfolgt verschlüsselt.
2025-06-26 18:17:44 - Neuer Client verbunden
2025-06-26 18:17:45 - → Modbus Diagnose-Funktion erkannt (Subfunction: 0x0000)
2025-06-26 18:17:45 - → Lokale Antwort auf Echo-Test (FC 0x08, Sub 0x0000)
2025-06-26 18:17:47 - Fehler beim Senden an RTU: Connection timed out

🔧 Optionale Erweiterungen
Vorschlag	Beschreibung
🔹 log_debug(), log_error()	Unterscheidung von Logtypen
🔹 Rotation via logrotate	Vermeidung von überlaufenden Logdateien
🔹 Remote-Logging via MQTT/HTTP	Übertragung der Logs an zentralen Server
🔹 JSON-Format	Strukturierte Logdaten für maschinelle Auswertung
🧩 Integration in Systemd

Falls systemd verwendet wird, kann das Logging auch an journalctl übergeben werden – z. B. durch:

logFilePath = /dev/stdout

Dann stehen alle Logs zentral per journalctl -u modbus_gateway.service zur Verfügung.

