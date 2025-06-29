❗ Kapitel 13: Fehlerbehandlung
🧩 Ziel der Fehlerbehandlung

Ein robustes Gateway muss fehlerhafte Zustände erkennen, protokollieren und – sofern möglich – automatisch beheben oder den Betrieb sicher beenden.
🔧 Kategorien von Fehlern
Kategorie	Beispiele	Behandlungsstrategie
🔌 Verbindungsfehler	MQTT-Server nicht erreichbar, RTU getrennt	Wiederverbindungsversuche
⚙️ Konfigurationsfehler	Fehlende/ungültige INI-Werte, nicht lesbare Dateien	Frühzeitiger Abbruch mit Log
🧱 Protokollfehler	Ungültige MBAP-/RTU-Rahmen, unzulässige Funktionen	Abbruch einzelner Anfrage
💥 Laufzeitfehler	Exceptions, System-Signale	sauberes shutdown_gateway()
🪓 MQTT-Verbindungsfehler
Fehlerfall:

MQTT error [-6]: Invalid (NULL) parameter

Behandlung:

    Try/Catch in init_mqtt()

    Logging der Fehlermeldung

    Rückgabe false → beendet main()

Beispiel:

catch (const mqtt::exception &e)
{
    log("Fehler bei MQTT-Verbindung: " + std::string(e.what()));
    return false;
}

🧱 Fehler bei Modbus-Übertragung
a) Fehler beim Senden an RTU

int sent = modbus_send_raw_request(rtu_ctx, full_request, full_len);
if (sent == -1)
{
    log("Fehler beim Senden an RTU: " + std::string(modbus_strerror(errno)));
    break;
}

b) Fehler bei Empfang

int res_len = modbus_receive_confirmation(rtu_ctx, response);
if (res_len == -1)
{
    failed_rtu_requests++;
    log("Fehler beim Empfangen von RTU: " + std::string(modbus_strerror(errno)));
    break;
}

📥 Fehlerhafte Konfiguration

In init_config():

if (ini_parse(config_path, config_handler, &config) < 0)
{
    log("Konnte Konfiguration nicht laden, nutze Defaults.");
    return false;
}

Erweiterung für TLS-Pfade:

if (!config.mqttCAFile.empty() && access(config.mqttCAFile.c_str(), R_OK) != 0)
    log("⚠️  Fehler: CA-Datei nicht lesbar: " + config.mqttCAFile);

🔄 Automatischer Reconnect bei MQTT

Der MQTT Callback behandelt:

    Verbindungsabbruch (connection_lost)

    Reconnect (ohne Parallelität durch Flag)

Beispiel:

void mqtt_callback::connection_lost(const std::string &cause)
{
    if (!reconnecting)
    {
        reconnecting = true;
        log("MQTT-Verbindung verloren: " + cause);
        std::thread([]()
        {
            while (run && !mqttClient->is_connected())
            {
                try { mqttClient->reconnect()->wait(); }
                catch (...) { std::this_thread::sleep_for(2s); }
            }
            reconnecting = false;
        }).detach();
    }
}

🔔 Behandlung von Signalen

In main():

signal(SIGINT, signal_handler);
signal(SIGTERM, signal_handler);

In signal_handler():

void signal_handler(int)
{
    run = false;
    log("Beende Gateway durch Signal");
}

→ Führt zum sauberen Beenden über shutdown_gateway()
🧪 Diagnose durch Statistik

Fehler werden zählbar gemacht:

failed_rtu_requests++;
total_dropped++;

Diese Werte werden regelmäßig als JSON via MQTT veröffentlicht:

{
  "tcp_requests": 122,
  "rtu_success": 118,
  "rtu_failures": 4,
  "dropped": 2,
  "echo_replies": 3
}

💡 Erweiterungsvorschläge
Vorschlag	Nutzen
📈 Fehlerrate pro Minute	Früherkennung von Störungen
📉 Reset bei zu vielen Fehlern	Auto-Recovery / Watchdog
🧩 Fehlercodes als MQTT-Topic	z. B. modbus_gateway/error mit Ursache & Zeitstempel


14. Systemd Integration
14.1 Service-Datei (/etc/systemd/system/modbus_gateway.service)

[Unit]
Description=Modbus TCP <-> RTU + MQTT Gateway
After=network.target

[Service]
ExecStart=/usr/local/bin/modbus_gateway /etc/modbus_gateway.ini
Restart=always
User=modbus
Group=modbus

[Install]
WantedBy=multi-user.target

