📊 Kapitel 10: Diagnose & Statistik
🧭 Ziel der Statistikfunktionen

Das Gateway protokolliert und veröffentlicht auf Wunsch intern wichtige Betriebskennzahlen. Diese dienen:

    der Laufzeitdiagnose,

    dem Debugging,

    der externen Visualisierung (z. B. via MQTT + Grafana).

📈 Verfügbare Zähler
Bezeichner	Bedeutung
total_tcp_requests	Anzahl aller Modbus TCP-Anfragen
successful_rtu_requests	Erfolgreich ausgeführte RTU-Anfragen
failed_rtu_requests	Fehlgeschlagene RTU-Kommunikationen
total_dropped	Abgelehnte/verlorene Verbindungen
total_echo	Lokal beantwortete Echo-Funktionsaufrufe
🧮 Datenstruktur im Code

Diese Zähler sind als std::atomic<uint64_t> deklariert, um Thread-Sicherheit bei gleichzeitiger Verarbeitung zu gewährleisten:

std::atomic<uint64_t> total_tcp_requests{0};
std::atomic<uint64_t> successful_rtu_requests{0};
std::atomic<uint64_t> failed_rtu_requests{0};
std::atomic<uint64_t> total_dropped{0};
std::atomic<uint64_t> total_echo{0};

🔁 Statistik-Thread

Ein Hintergrund-Thread wird bei Programmstart aktiviert, wenn in der Konfiguration gesetzt:

mqttStatsInterval = 60

Dieser Thread publiziert regelmäßig die aktuellen Statistikdaten via MQTT (siehe unten).

Beispielcode:

if (config.mqttStatsInterval > 0)
{
    stats_thread = std::thread([] {
        while (run)
        {
            std::this_thread::sleep_for(std::chrono::seconds(config.mqttStatsInterval));
            if (mqttClient && mqttClient->is_connected())
                publish_stats();
        }
    });
}

📤 MQTT Statistikmeldung

Die Statistiken werden zyklisch über das MQTT-Topic modbus/stats (konfigurierbar) im JSON-Format gesendet:

Beispielausgabe:

{
  "tcp_requests": 2035,
  "rtu_success": 1992,
  "rtu_failures": 9,
  "dropped": 3,
  "echo_replies": 31
}

Topic-Beispiel:

mqttTopicPrefix = modbus/

Ergibt z. B.: modbus/stats
📮 Statistik auf Anfrage senden

Optional kann das Gateway auf das Topic modbus/command/stats hören (falls aktiviert), um auf Befehl die aktuellen Statistiken zu veröffentlichen.

Das funktioniert durch zusätzliche Subscription + Topic-Abfrage in der Callback-Funktion.
♻️ Zähler zurücksetzen

Über Diagnosefunktion 0x08/0x000A (Clear Counters) lassen sich Zähler auf 0 zurücksetzen – lokal beantwortet ohne RTU.

Beispiel:

Client sendet Diagnose-Anfrage (0x08 00 0A 00 00)
→ Zähler im Gateway werden genullt
← Antwort erfolgt sofort vom Gateway

🧪 Beispiel: MQTT + Grafana

Die Werte können per MQTT-Bridge (z. B. Node-RED, Telegraf) in eine Zeitreihendatenbank überführt und dort visualisiert werden.
Mögliche Tools:

    InfluxDB

    Grafana

    Node-RED

🛠️ Konfigurationsoptionen

mqttStatsInterval = 30        ; Sekunden – 0 = deaktiviert
mqttTopicPrefix = modbus/

🧼 Beenden und Cleanup

Der Statistik-Thread wird im shutdown_gateway() über join() beendet, damit keine Zombie-Threads entstehen.

