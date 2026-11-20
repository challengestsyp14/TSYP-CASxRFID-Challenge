from dataclasses import dataclass
from datetime import datetime, timedelta, timezone
from app.custody import apply_custody, liable

A, B, C = "A", "B", "C"
ROLES = {A: "warehouse", B: "driver", C: "client"}
BASE = datetime(2026, 1, 1, tzinfo=timezone.utc)

@dataclass
class Ev:
    seq: int
    uptime_ms: int
    type: str
    height_m: float | None = None
    badge_id: str | None = None
    holder_badge_id: str | None = None
    severity: str | None = None
    expected: bool = False
    event_time: datetime | None = None

def tap(seq, t, badge):
    return Ev(seq, t, "tap", badge_id=badge, event_time=BASE + timedelta(milliseconds=t))

def ev(seq, t, type, height=None):
    return Ev(seq, t, type, height_m=height, event_time=BASE + timedelta(milliseconds=t))

def run(*events):
    result, handovers = apply_custody(list(events), ROLES)
    return {e.seq: e for e in result}, handovers

def test_1_drop_after_tap_b():
    r, _ = run(tap(1, 1000, B), ev(2, 5000, "drop", 0.8))
    assert r[2].holder_badge_id == B

def test_2_drop_before_any_tap():
    r, _ = run(ev(1, 1000, "drop", 0.8))
    assert r[1].holder_badge_id is None

def test_3_tap_a_tap_b_then_drop():
    r, _ = run(tap(1, 1000, A), tap(2, 5000, B), ev(3, 9000, "drop", 0.8))
    assert r[3].holder_badge_id == B

def test_4_same_badge_twice_within_2s_ignored():
    _, handovers = run(tap(1, 1000, A), tap(2, 5000, B), tap(3, 6000, B))
    assert handovers == [(1, A), (2, B)]

def test_5_tamper_while_client_holds_is_expected():
    r, _ = run(tap(1, 1000, C), ev(2, 3000, "tamper"))
    assert r[2].expected is True
    assert r[2].severity == "info"

def test_6_tamper_while_driver_holds_is_severe():
    r, _ = run(tap(1, 1000, B), ev(2, 3000, "tamper"))
    assert r[2].severity == "severe"
    assert liable(r.values()) == [{"badge_id": B, "reason": "tamper"}]

def test_9_temp_breach_after_tap_b_is_warning():
    r, _ = run(tap(1, 1000, B), ev(2, 3000, "temp_breach"))
    assert r[2].severity == "warning"
    assert r[2].holder_badge_id == B


# ---- Tests 7, 8 and 10: the full upload, with a temporary in-memory database ----
from fastapi.testclient import TestClient
from sqlalchemy import create_engine, select
from sqlalchemy.orm import sessionmaker
from sqlalchemy.pool import StaticPool
from app.main import app
from app.models import Base, Event
from app.routes import get_db

def make_client():
    engine = create_engine("sqlite://", connect_args={"check_same_thread": False},
                           poolclass=StaticPool)
    Base.metadata.create_all(engine)
    Session = sessionmaker(bind=engine, autoflush=False)

    def test_db():
        db = Session()
        try:
            yield db
        finally:
            db.close()

    app.dependency_overrides[get_db] = test_db
    return TestClient(app), Session

def rows(Session):
    db = Session()
    out = [(e.seq, e.type, e.holder_badge_id, e.severity)
           for e in db.scalars(select(Event).order_by(Event.seq))]
    db.close()
    return out

def upload(client, events, at=20000):
    return client.post("/api/upload", json={
        "box_id": "B001", "uploaded_at_uptime_ms": at, "events": events})

STORY = [
    {"seq": 1, "uptime_ms": 1000, "type": "tap", "badge_id": "A"},
    {"seq": 2, "uptime_ms": 5000, "type": "tap", "badge_id": "B"},
    {"seq": 3, "uptime_ms": 9000, "type": "drop", "height_m": 0.8},
    {"seq": 4, "uptime_ms": 12000, "type": "tamper"},
]

def test_7_same_upload_twice_changes_nothing():
    client, Session = make_client()
    assert upload(client, STORY).json() == {"ack_seq": 4}
    first = rows(Session)
    assert upload(client, STORY).json() == {"ack_seq": 4}
    assert rows(Session) == first
    assert len(first) == 4

def test_8_split_upload_gives_same_result():
    client, Session = make_client()
    upload(client, STORY)
    one_upload = rows(Session)

    client, Session = make_client()
    upload(client, STORY[:2], at=6000)
    upload(client, STORY[2:], at=20000)
    assert rows(Session) == one_upload

def test_10_event_from_before_reboot_gets_receive_time():
    client, Session = make_client()
    before = datetime.now(timezone.utc).replace(tzinfo=None)
    # uptime 50 s but the tag uploads at 3 s: it rebooted in between
    upload(client, [{"seq": 1, "uptime_ms": 50000, "type": "drop", "height_m": 0.5}], at=3000)
    after = datetime.now(timezone.utc).replace(tzinfo=None)

    db = Session()
    e = db.scalars(select(Event)).one()
    db.close()
    assert before <= e.event_time <= after
