# Parcel Custody Tag

Smart tag inside a parcel: records drops, impacts, lid openings and temperature,
knows who held the box (RFID badge taps), and shows who is liable on a dashboard.

## First-time setup (once per laptop)

Backend (Python 3.11):
    cd backend
    python -m venv venv
    venv\Scripts\activate
    pip install -r requirements.txt

Dashboard (Node.js LTS):
    cd dashboard
    npm install

Firewall, so the tag can reach the laptop (PowerShell as administrator):
    New-NetFirewallRule -DisplayName "Parcel backend 8000" -Direction Inbound -Protocol TCP -LocalPort 8000 -Action Allow

## Start everything

From the repo root:
    .\start.ps1

- Backend: http://localhost:8000 (API docs at /docs)
- Dashboard: http://localhost:5173

## Restart the demo

    .\reset.ps1

Deletes all events. Badges A, B and C stay registered.

## Test without the tag

    cd backend
    venv\Scripts\activate
    python simulate_tag.py              # demo scenario for box B001
    python simulate_tag.py --box B002   # a second box
    python simulate_tag.py --repeat     # send twice, no duplicates

## Connect the real tag

1. Connect the laptop and the tag to the same phone hotspot.
2. Find the laptop IP: run ipconfig, look under the Wi-Fi adapter, "IPv4 Address".
3. The tag posts to http://<laptop-IP>:8000/api/upload
4. Check from a phone on the hotspot: http://<laptop-IP>:8000/api/health

## Run the tests

    cd backend
    venv\Scripts\activate
    python -m pytest -v
