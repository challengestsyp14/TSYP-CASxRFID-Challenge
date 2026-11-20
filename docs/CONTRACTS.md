# Parcel Custody Tag — system contracts

This is a student project: a smart parcel custody tag.
An ESP-32 devkitv1 tag inside a parcel detects handling events (drop, impact, upside-down, lid opened = tamper, temperature out of range = temp_breach) using an IMU, a light sensor, a DHT11 temperature and humidity sensor and an ML model. An RFID reader mounted on the tag logs badge taps: a tap means that person now holds the box. The tag uploads its event log over Wi-Fi to a FastAPI backend using HTTP POST. The backend converts times, assigns each event to the badge holder at that moment, and a React dashboard shows each box's custody timeline and who is liable.

## Folders
- backend/ — backend developer (FastAPI, custody logic, simulator)
- dashboard/ — frontend developer (React + Vite + Tailwind)
- ml/ — frontend developer (Edge Impulse notes, figures)
- tag/ — firmware developer (ESP-32 firmware)
- docs/ — report figures

## CONTRACTS

### Tag event
{"box_id": "B001", "seq": 42, "uptime_ms": 123456, "type": "drop", "conf": 0.93, "height_m": 0.8, "temp_c": 24.1, "hum_pct": 51, "badge_id": null}
- type is one of: tap, drop, impact, upside_down, tamper, temp_breach.
- seq starts at 1, goes up by 1 per event and is never reused, even after a reboot.
- uptime_ms is milliseconds since the tag booted. The tag does not need a real clock.
- Fields that don't apply are null. tap sets badge_id; drop sets height_m. Every event also carries the latest temp_c and hum_pct from the DHT11.
- temp_breach is raised when temp_c leaves the safe range. Starting values: above 30 °C or below 5 °C (to be tuned).
- badge_id is the RFID card UID as an uppercase hex string with no separators, for example 04A3B2C1 (format to be confirmed on the hardware).

### Upload (tag to backend)
POST /api/upload
{"box_id": "B001", "uploaded_at_uptime_ms": 130000, "events": [ {"seq": 41, "...": "..."}, {"seq": 42, "...": "..."} ]}
Response 200: {"ack_seq": 42}
Optional "profile" in the upload body: {"fragile": true, "temp_mode": "room", "temp_min_c": 5.0, "temp_max_c": 30.0}. temp_mode is cold, room or medication; temp_min_c and temp_max_c are the tag's temp_breach limits for that mode. The tag sends it with every upload, and on its own (with "events": []) after a restart or when the profile changes. The backend keeps the latest one per box. It is display only: severity rules are unchanged.
The tag deletes events up to ack_seq only after this response. Uploading the same events twice changes nothing (unique on box_id + seq).

### Time conversion (done by the backend)
event_time = server_receive_time − (uploaded_at_uptime_ms − event.uptime_ms)

### Custody and severity rules
- Events of one box are processed in seq order.
- A tap makes its badge_id the holder from that moment. The previous holder's custody ends. The same badge tapping again within 2 s is ignored.
- Every other event gets holder_badge_id = the current holder. If nobody has tapped yet, it is null and shown as "unassigned".
- A tamper while the holder's role is client is expected = true, so it is not a fault.
- Severity: drop of 0.30 m or more, and impact, are severe. drop below 0.30 m, upside_down and temp_breach are warning. A tamper that is not expected is severe. tap is info. (Thresholds to be tuned on the hardware.)
- A box's liable party is the holder of each non-expected warning or severe event. The dashboard lists every event with its holder.

### REST API (dashboard reads)
| Endpoint | Returns |
|---|---|
| GET /api/boxes | list of {box_id, status, last_event_time, current_holder, profile}; status is ok, warning or severe; profile is the object above or null |
| GET /api/boxes/{box_id}/timeline | {box_id, status, profile, liable: [{badge_id, name, reason}], events: [{seq, time, type, severity, height_m, temp_c, hum_pct, holder: {badge_id, name, role}, expected}]}; time is ISO 8601 |
| GET /api/handlers | list of {badge_id, name, role}; role is warehouse, driver or client |
| POST /api/handlers | body {badge_id, name, role}; registers a badge |
| POST /api/reset | demo only: deletes all events so the demo can restart |
| GET /api/health | {"status": "ok"} |

The dashboard polls every 2 s. CORS is enabled. An unknown badge is shown with the name "Unknown badge".
Agreed with the backend developer: current_holder is an object {badge_id, name, role} or null; an unassigned event has holder: null; an expected tamper has severity info.

### ML contract (ML and firmware)
- A window is 2 s at 100 Hz: 200 samples of 6 values (ax, ay, az, gx, gy, gz).
- Training is done in Edge Impulse, which also computes the features on the tag, so there is no separate features file.
- The Edge Impulse project has the labels carried, impact, still and upside_down, and is exported as an Arduino library for the ESP-32 devkitv1.
- `still` (box not moving) produces no event, like carried.
- A drop is detected in the firmware by the free-fall rule, not by the model: |a| < 0.5 g for at least 15 samples at 100 Hz, followed by a spike |a| > 30 m/s² (about 3 g); a free fall without that spike is not a drop. The free-fall time t also gives height_m = ½gt². When a drop is detected, the model's impact for that window is ignored.
- Event types in the Tag event contract are unchanged (drop is still an event type).
- The firmware streams the IMU over USB serial, one line per sample as ax,ay,az,gx,gy,gz at 100 Hz, so the Edge Impulse data forwarder can collect labeled data.
