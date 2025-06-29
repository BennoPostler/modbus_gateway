🚀 Kapitel 15: Deployment

In diesem Kapitel beschreiben wir den empfohlenen Ablauf für die Bereitstellung (Deployment) des Gateways auf einem Zielsystem – typischerweise einem Raspberry Pi 5 mit Raspberry Pi OS (64-bit).
📦 Ziel des Deployments

Das Deployment-Paket soll:

    das Kompilat für aarch64 enthalten

    die Konfigurationsdatei (modbus_gateway.ini) beilegen

    die systemd-Service-Datei bereitstellen

    optional: ein Installationsskript mit Setup-Schritten

📁 Empfohlene Struktur des Release-Verzeichnisses

modbus_gateway_release/
├── modbus_gateway              # ⚙️ Kompilierte Binärdatei (aarch64)
├── modbus_gateway.ini          # 🛠 Konfigurationsdatei
├── modbus_gateway.service      # 🧩 systemd-Service
├── install.sh                  # 🧰 Installationsskript
└── README.md                   # 📘 Dokumentation

🧰 Beispiel: Installationsskript install.sh

#!/bin/bash

echo "📦 Installiere Modbus Gateway..."

# Benutzer anlegen, falls nicht vorhanden
if ! id "modbusgw" &>/dev/null; then
    echo "👤 Erstelle Benutzer 'modbusgw'"
    sudo useradd --system --no-create-home --shell /usr/sbin/nologin modbusgw
fi

# Dateien kopieren
echo "📁 Kopiere Dateien..."
sudo cp modbus_gateway /usr/local/bin/
sudo cp modbus_gateway.ini /etc/
sudo cp modbus_gateway.service /etc/systemd/system/

# Berechtigungen setzen
sudo chown root:root /usr/local/bin/modbus_gateway
sudo chmod 755 /usr/local/bin/modbus_gateway

sudo chmod 644 /etc/modbus_gateway.ini
sudo chmod 644 /etc/systemd/system/modbus_gateway.service

# Benutzer zur dialout-Gruppe hinzufügen (für /dev/ttyUSB0 Zugriff)
sudo usermod -aG dialout modbusgw

# Dienst aktivieren und starten
sudo systemctl daemon-reload
sudo systemctl enable modbus_gateway
sudo systemctl start modbus_gateway

echo "✅ Modbus Gateway wurde installiert und gestartet."

📦 Paket-Erstellung mit tar.gz

Auf deinem Buildsystem (z. B. Fedora, Cross-Compiled):

mkdir modbus_gateway_release
cp modbus_gateway modbus_gateway_release/
cp modbus_gateway.ini modbus_gateway_release/
cp modbus_gateway.service modbus_gateway_release/
cp install.sh modbus_gateway_release/
cp README.md modbus_gateway_release/

tar -czvf modbus_gateway.tar.gz modbus_gateway_release/

→ Dieses Archiv lässt sich direkt auf dem Pi entpacken und ausführen.
🧪 Testlauf auf Zielsystem

scp modbus_gateway.tar.gz pi@raspberrypi:/tmp/
ssh pi@raspberrypi

cd /tmp
tar -xzf modbus_gateway.tar.gz
cd modbus_gateway_release
chmod +x install.sh
./install.sh

Danach prüfen:

sudo systemctl status modbus_gateway

🔒 Sicherheitshinweise

    Gib nur Lesezugriff auf modbus_gateway.ini

    Nutze User=modbusgw in der Service-Datei

    Verwende TLS für MQTT, wo möglich

    Nutze Firewalls für TCP-Port 502, falls öffentlich erreichbar

