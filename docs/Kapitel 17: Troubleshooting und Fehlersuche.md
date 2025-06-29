🧯 Kapitel 17: Troubleshooting & Fehlersuche

Dieses Kapitel hilft dir bei der systematischen Diagnose, wenn etwas nicht wie erwartet funktioniert. Es enthält typische Fehlersymptome, mögliche Ursachen und empfohlene Lösungswege.
🔌 17.1 Keine Verbindung zum Modbus-RTU-Gerät

Symptome:

    Es erscheinen regelmäßig Fehler beim Senden an RTU oder Fehler beim Empfangen von RTU

    Alle Anfragen schlagen fehl

Ursachen & Lösungen:
Ursache	Lösung
Falscher serieller Port	Prüfen: device = /dev/ttyUSB0 oder z. B. /dev/ttyAMA0
Baudrate/Parität falsch	Anpassen auf Gerät (z. B. 19200 bps, 8N1)
Kein Abschlusswiderstand	Bei RS485 prüfen
Busspannung fehlt	GND-/RS485-Leitungen checken
Kein oder falscher Slave	unit-ID überprüfen
🌐 17.2 Kein Zugriff über Modbus TCP

Symptome:

    Verbindung via mbpoll oder SCADA schlägt fehl

    Port 502 nicht erreichbar

Prüfpunkte:

    Läuft das Gateway? → ps aux | grep modbus_gateway

    Lauscht es auf dem TCP-Port? → netstat -an | grep 502

    Firewall geöffnet? → sudo ufw allow 502/tcp

    Wird start_tcp_server() erfolgreich ausgeführt?

📡 17.3 MQTT-Verbindung schlägt fehl

Logmeldungen:

    Fehler bei MQTT-Verbindung: MQTT error [-6]: Invalid (NULL) parameter

    connect error: Unknown host oder TLS-Fehler

Maßnahmen:

    Broker-Adresse korrekt? → broker = 192.168.0.82

    Port geöffnet? (meist 1883 / 8883 für TLS)

    TLS aktiv, aber ohne gültige Zertifikate? → mqttUseTLS = false testweise setzen

    Broker online? → mosquitto_sub -h ... oder telnet 192.168.0.82 1883

🧪 17.4 Diagnosefunktion funktioniert nicht

Symptom: mbpoll mit Function Code 08 liefert keine Antwort.

Lösungen:

    Gateway unterstützt nur bestimmte Subfunktionen:

        0x0000 (Echo-Test)

        0x000A (Zähler zurücksetzen)

        0x000B (Bus-Message-Count)

        0x000C (Fehlerzähler)

    Für nicht unterstützte wird korrekt keine lokale Antwort gegeben → Anfrage geht an RTU

📉 17.5 Statistik wird nicht gesendet

Symptom: MQTT-Topic modbus/stats bleibt leer.

Checkliste:
Prüfen	Korrektur
mqttStatsInterval > 0 gesetzt?	Im INI setzen, z. B. mqttStatsInterval = 60
mqttClient->is_connected()?	MQTT läuft und verbunden
Wird publish_stats() aufgerufen?	Ja – über Thread in main()
Debuglog aktiviert?	debug_hex_dump = true liefert mehr Details
🔁 17.6 Gateway hängt oder beendet sich

Symptome:

    Keine Reaktion mehr

    Crash bei vielen gleichzeitigen Verbindungen

Mögliche Ursachen:

    Stacküberlauf in Threads → zu kleine Puffer

    Ungültige RTU-Antworten → besser mit dump_hex() debuggen

    race conditions durch fehlende std::atomic beim Statistikzähler

    TLS: Nicht-blockierende Fehler bei Verbindungsversuch

