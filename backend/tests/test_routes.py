from test_custody import make_client, upload

STORY = [
    {"seq": 1, "uptime_ms": 1000, "type": "tap", "badge_id": "B"},
    {"seq": 2, "uptime_ms": 4000, "type": "drop", "height_m": 0.8},
    {"seq": 3, "uptime_ms": 8000, "type": "tap", "badge_id": "C"},
]

def add_people(client):
    client.post("/api/handlers", json={"badge_id": "B", "name": "Driver B", "role": "driver"})
    client.post("/api/handlers", json={"badge_id": "C", "name": "Client C", "role": "client"})

def test_handlers_post_then_get():
    client, _ = make_client()
    add_people(client)
    names = {h["badge_id"]: h["name"] for h in client.get("/api/handlers").json()}
    assert names == {"B": "Driver B", "C": "Client C"}

def test_boxes_shows_status_and_current_holder():
    client, _ = make_client()
    add_people(client)
    upload(client, STORY)
    boxes = client.get("/api/boxes").json()
    assert len(boxes) == 1
    assert boxes[0]["status"] == "severe"
    assert boxes[0]["current_holder"] == {"badge_id": "C", "name": "Client C", "role": "client"}
    assert boxes[0]["last_event_time"].endswith("Z")

def test_timeline_liable_has_holder_name():
    client, _ = make_client()
    add_people(client)
    upload(client, STORY)
    t = client.get("/api/boxes/B001/timeline").json()
    assert t["liable"] == [{"badge_id": "B", "name": "Driver B", "reason": "drop 0.8 m"}]
    assert [e["holder"]["name"] for e in t["events"]] == ["Driver B", "Driver B", "Client C"]

def test_timeline_unknown_box_is_404():
    client, _ = make_client()
    assert client.get("/api/boxes/B999/timeline").status_code == 404

def test_reset_clears_everything():
    client, _ = make_client()
    upload(client, STORY)
    client.post("/api/reset")
    assert client.get("/api/boxes").json() == []

def test_registering_client_later_makes_tamper_expected():
    client, _ = make_client()
    upload(client, [
        {"seq": 1, "uptime_ms": 1000, "type": "tap", "badge_id": "C"},
        {"seq": 2, "uptime_ms": 3000, "type": "tamper"},
    ])
    add_people(client)  # C becomes a client after the upload
    t = client.get("/api/boxes/B001/timeline").json()
    assert t["events"][1]["expected"] is True
    assert t["status"] == "ok"

FRAGILE_ROOM = {"fragile": True, "temp_mode": "room", "temp_min_c": 5.0, "temp_max_c": 30.0}

def send(client, events, profile=None, at=20000):
    body = {"box_id": "B001", "uploaded_at_uptime_ms": at, "events": events}
    if profile is not None:
        body["profile"] = profile
    return client.post("/api/upload", json=body)

def test_profile_shown_in_boxes_and_timeline():
    client, _ = make_client()
    assert send(client, STORY, FRAGILE_ROOM).status_code == 200
    assert client.get("/api/boxes").json()[0]["profile"] == FRAGILE_ROOM
    assert client.get("/api/boxes/B001/timeline").json()["profile"] == FRAGILE_ROOM

def test_no_profile_is_null():
    client, _ = make_client()
    upload(client, STORY)
    assert client.get("/api/boxes").json()[0]["profile"] is None
    assert client.get("/api/boxes/B001/timeline").json()["profile"] is None

def test_profile_only_upload_with_no_events():
    client, _ = make_client()
    send(client, STORY, FRAGILE_ROOM)
    cold = {"fragile": False, "temp_mode": "cold", "temp_min_c": 0.0, "temp_max_c": 10.0}
    r = send(client, [], cold, at=30000)
    assert r.status_code == 200 and r.json() == {"ack_seq": 3}
    assert client.get("/api/boxes").json()[0]["profile"] == cold
    assert len(client.get("/api/boxes/B001/timeline").json()["events"]) == 3

def test_profile_only_upload_from_a_new_box():
    client, _ = make_client()
    r = send(client, [], FRAGILE_ROOM)
    assert r.json() == {"ack_seq": 0}
    box = client.get("/api/boxes").json()[0]
    assert box["profile"] == FRAGILE_ROOM and box["status"] == "ok"

def test_upload_without_profile_keeps_the_last_one():
    client, _ = make_client()
    send(client, STORY, FRAGILE_ROOM)
    upload(client, [{"seq": 4, "uptime_ms": 15000, "type": "tamper"}])
    assert client.get("/api/boxes").json()[0]["profile"] == FRAGILE_ROOM

def test_bad_temp_mode_is_rejected():
    client, _ = make_client()
    bad = dict(FRAGILE_ROOM, temp_mode="frozen")
    assert send(client, STORY, bad).status_code == 422

def test_reset_clears_the_profile():
    client, _ = make_client()
    send(client, STORY, FRAGILE_ROOM)
    client.post("/api/reset")
    send(client, [], None)
    assert client.get("/api/boxes").json()[0]["profile"] is None
