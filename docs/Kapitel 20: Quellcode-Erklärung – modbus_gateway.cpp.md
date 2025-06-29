---
title: Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi
layout: default
---

🧬 Kapitel 20: Quellcode-Erklärung – modbus_gateway.cpp

Dieses Kapitel enthält eine ausführliche Erklärung des zentralen Quellcodes. Ziel ist es, alle relevanten Logikabschnitte nachvollziehbar zu beschreiben, damit Entwickler sich schnell einarbeiten und das Verhalten anpassen können.
🧾 20.1 Includes & globale Initialisierung

#include <modbus/modbus.h>
#include <mqtt/async_client.h>
#include <fstream>
#include <thread>
#include <atomic>
#include <csignal>
#include <vector>
#include <sstream>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "ini.h"
#include "config_structs.h"

    Modbus: libmodbus für TCP/RTU-Kommunikation

    MQTT: Eclipse Paho MQTT C++ Client

    Threading: std::thread, std::atomic

    INI: Eigenes INI-Parsing-Modul

    Konfiguration: Struktur zur zentralen Konfigurationshaltung

Globale Variablen wie z. B. std::atomic<uint64_t> total_tcp_requests; werden deklariert, um Statistik zu führen.
🔔 20.2 Signalbehandlung

void signal_handler(int signal)
{
    run = false;
}

    Abfangen von SIGINT und SIGTERM

    Beendet alle Hauptschleifen (u. a. Statistik-Thread, Client-Threads)

📦 20.3 main() – Programmstart

int main(int argc, char *argv[])

Hauptaufgaben:

    Parameter analysieren:

        --help, --version, --dry-run

    Initialisierung:

        init_config()

        init_mqtt()

        init_modbus()

        start_tcp_server()

    Statistik-Thread starten

    Client-Anfragen akzeptieren

    Cleanup mit shutdown_gateway()

Wenn --dry-run gesetzt ist, werden nur Konfigurationswerte geprüft und die Software beendet sich danach automatisch.
🌐 20.4 TCP-Verarbeitung: accept_clients()

void accept_clients()

    Lauscht auf eingehende TCP-Verbindungen (Standard: Port 5020)

    Für jeden Client wird std::thread mit handle_client() gestartet

    Jeder Thread führt TCP→RTU→TCP-Übersetzung durch

🔁 20.5 Modbus-Verarbeitung: handle_client()

Der wichtigste Teil der Anwendung.
Ablauf:

    TCP-Query via modbus_receive()

    MBAP-Header analysieren (transaction_id, unit_id)

    Bei Funktionscode 0x08:

        Subfunktion 0x0000 (Echo): lokal beantworten

        Subfunktion 0x000B (Anfragezähler): lokal simulieren

    PDU extrahieren, RTU-Request zusammensetzen

    Senden via modbus_send_raw_request()

    Antwort empfangen via modbus_receive_confirmation()

    MBAP-Header rekonstruieren für TCP

    Antwort an Client senden

Besonderheiten:

    Fehlererkennung mit Logging

    Debug-Hexdump bei Bedarf aktivierbar

    Zähler für erfolgreiche/fehlgeschlagene Modbus-Antworten

📊 20.6 Statistik-Thread

std::thread stats_thread;

Startet im Hintergrund, wenn mqttStatsInterval > 0 gesetzt ist.
Sendet regelmäßig:

{
  "tcp_requests": 42,
  "rtu_success": 40,
  "rtu_failures": 2,
  "dropped": 0,
  "echo_replies": 1
}

→ Ziel-Topic: modbus/stats
📤 20.7 publish_stats() (on-demand per MQTT)

Diese Funktion wird aufgerufen, wenn über das Topic modbus/command/stats eine Anforderung kommt.

void publish_stats()

    Baut ein JSON mit aktuellen Werten

    Sendet via mqttClient->publish(...)

📑 20.8 Shutdown & Cleanup

void shutdown_gateway()

Führt aus:

    Alle client_threads.join()

    Statistik-Thread join()

    MQTT clean disconnect

    Modbus-RTU-Verbindung schließen

    Socket schließen

    Logdatei abschließen

📦 20.9 Optional: TLS (MQTT)

Wenn mqttUseTLS=true gesetzt ist, werden folgende TLS-Felder genutzt:

    mqttCAFile

    mqttCertFile

    mqttKeyFile

    mqttInsecure (Zertifikatsprüfung deaktivieren)

Wird direkt in init_mqtt() ausgewertet und an die mqtt::ssl_options übergeben.
🧪 20.10 Dry-Run & Konfiguration

bool dry_run = false;

    Parameter --dry-run oder --validate-only

    Lädt die Konfiguration, prüft Datei-Zugriffe und TLS-Dateien

    Beendet sich ohne Modbus-/MQTT-/TCP-Verbindung

