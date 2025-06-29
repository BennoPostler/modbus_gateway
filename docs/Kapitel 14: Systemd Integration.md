🧩 Kapitel 14: Systemd Integration

Dieses Kapitel beschreibt die Integration des Modbus-Gateways als systemd-Service unter Linux – insbesondere für Raspberry Pi OS (Debian-basiert), aber auch allgemein unter allen Systemd-fähigen Distributionen wie Fedora, Ubuntu oder Arch.
✅ Ziel

Das Modbus-Gateway soll:

    automatisch beim Systemstart starten

    bei einem Absturz automatisch neu starten

    unter einem eigenen Benutzer mit minimalen Rechten laufen

    Logs gezielt in systemd-journal oder separate Dateien schreiben

    sich sauber beenden lassen, z. B. per systemctl stop modbus_gateway

📁 Service-Datei: /etc/systemd/system/modbus_gateway.service

[Unit]
Description=Modbus TCP <-> RTU + MQTT Gateway
After=network.target

[Service]
Type=simple
User=modbusgw
Group=modbusgw
ExecStart=/usr/local/bin/modbus_gateway /etc/modbus_gateway.ini
WorkingDirectory=/usr/local/bin
Restart=on-failure
RestartSec=5
StandardOutput=journal
StandardError=journal

# Ressourcenlimits (optional)
LimitCORE=infinity
LimitNOFILE=4096

[Install]
WantedBy=multi-user.target

👤 Benutzer & Berechtigungen

Aus Sicherheitsgründen sollte das Gateway nicht als root laufen. Lege einen dedizierten Benutzer an:

sudo useradd --system --no-create-home --shell /usr/sbin/nologin modbusgw

Stelle sicher, dass:

    /usr/local/bin/modbus_gateway vom Benutzer modbusgw ausführbar ist

    /etc/modbus_gateway.ini lesbar ist

    ggf. die Modbus-Seriellschnittstelle (z. B. /dev/ttyUSB0) lesbar ist:

sudo usermod -aG dialout modbusgw
sudo chmod 660 /dev/ttyUSB0

🛠️ Installation & Aktivierung

    Installiere das Binary:

sudo cp modbus_gateway /usr/local/bin/
sudo chmod 755 /usr/local/bin/modbus_gateway

    Kopiere die .ini-Datei:

sudo cp modbus_gateway.ini /etc/

    Erstelle die service-Datei:

sudo nano /etc/systemd/system/modbus_gateway.service

    Aktivieren und starten:

sudo systemctl daemon-reexec       # Nur wenn systemd frisch konfiguriert
sudo systemctl daemon-reload       # Dienstdefinitionen neu einlesen
sudo systemctl enable modbus_gateway
sudo systemctl start modbus_gateway

    Status prüfen:

sudo systemctl status modbus_gateway
journalctl -u modbus_gateway -f

♻️ Automatischer Neustart bei Fehlern

Dank Restart=on-failure wird der Dienst bei Abstürzen automatisch neu gestartet. Du kannst dieses Verhalten anpassen:
Option	Beschreibung
Restart=always	Neustart bei jedem Exit-Code
Restart=on-abort	Nur bei Signals wie SIGABRT
RestartSec=5	5 Sekunden Pause vor Neustart
📃 Log-Ausgabe

Die Anwendung schreibt standardmäßig zusätzlich in logFilePath (aus der .ini), aber systemd protokolliert auch:

journalctl -u modbus_gateway.service

Wenn du willst, kannst du den Logausgabe-Modus umstellen:

StandardOutput=file:/var/log/modbus_gateway.log
StandardError=inherit

Dann ggf. Logrotate einrichten (→ Kapitel 15).
15. Deployment
15.1 Verzeichnisse
Zweck	Pfad
Konfig	/etc/modbus_gateway.ini
Logs	/var/log/modbus_gateway.log
Service	/etc/systemd/system/...

