---
title: "Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi"
layout: default
---

🔐 Kapitel 11: TLS & Sicherheit
🧭 Ziel der TLS-Unterstützung

Das Gateway kann – optional – verschlüsselte TLS-Verbindungen zu einem MQTT-Broker aufbauen. Dies erhöht:

    Vertraulichkeit (Inhalte werden verschlüsselt)

    Integrität (Abwehr von Man-in-the-Middle-Angriffen)

    Authentizität (Client und/oder Broker-Zertifikate)

🔧 Aktivierung von TLS

Die TLS-Funktionalität wird über die Konfigurationsdatei aktiviert:

mqttUseTLS     = true
mqttCAFile     = /etc/ssl/certs/ca-certificates.crt
mqttCertFile   = /etc/modbus_gateway/client.crt
mqttKeyFile    = /etc/modbus_gateway/client.key
mqttInsecure   = false

Erklärung der Parameter:
Name	Beschreibung
mqttUseTLS	TLS aktivieren (true/false)
mqttCAFile	CA-Zertifikat zur Broker-Verifizierung
mqttCertFile	Optionales Client-Zertifikat (für gegenseitige Authentifizierung)
mqttKeyFile	Schlüssel zum Zertifikat
mqttInsecure	Deaktiviert CA-Prüfung (unsicher, aber hilfreich bei Tests)
🔐 TLS-Handshake-Verhalten

    Bei aktiviertem TLS wird intern der mqtt::ssl_options Block gesetzt und dem mqtt::connect_options hinzugefügt.

    Die Kommunikation wird dann automatisch über SSL abgewickelt.

mqtt::ssl_options sslopts;
sslopts.set_trust_store(config.mqttCAFile);
sslopts.set_key_store(config.mqttCertFile);
sslopts.set_private_key(config.mqttKeyFile);
sslopts.set_enable_server_cert_auth(!config.mqttInsecure);

connOpts.set_ssl(sslopts);

🛠️ Fehlersuche bei TLS-Problemen

Typische Ursachen für Verbindungsfehler:

    CA-Datei fehlt oder ist nicht lesbar:
    → Prüfe Pfad und Berechtigungen.

    Client-Zertifikat + Key fehlen bei Auth-Zwang:
    → MQTT-Broker verlangt Client-Zertifikat.

    Zertifikatsprüfung schlägt fehl:
    → Setze testweise mqttInsecure = true, um das zu verifizieren.

📄 Zertifikate erzeugen (Beispiel)
🧪 Selbstsigniertes Zertifikat für Testbetrieb:

openssl req -x509 -newkey rsa:2048 -nodes -keyout client.key -out client.crt -days 365

Dann in modbus_gateway.ini eintragen:

mqttCertFile = /pfad/zum/client.crt
mqttKeyFile  = /pfad/zum/client.key
mqttInsecure = true

🧼 Fallback zu unverschlüsselt

Wenn mqttUseTLS = false gesetzt ist, wird keine TLS-Verschlüsselung aktiviert. Die Verbindung läuft dann über Standard-Port 1883 oder benutzerdefiniert.
🧮 Integration in init_mqtt()

Die TLS-Parameter werden nur gesetzt, wenn mqttUseTLS == true. Andernfalls erfolgt der Verbindungsaufbau ohne Verschlüsselung.

Fehler beim Aufbau werden geloggt und führen zum Abbruch des Startvorgangs.
🔒 Empfehlung

In produktiven Umgebungen sollte TLS immer aktiviert sein. Besonders bei:

    Cloud-Deployments

    öffentlich erreichbaren MQTT-Brokern

    integritätskritischen Automatisierungslösungen

