from datetime import datetime, timedelta, timezone
from typing import Literal
from fastapi import APIRouter, Depends
from pydantic import BaseModel
from sqlalchemy import select, func, delete
from sqlalchemy.orm import Session
from app.models import SessionLocal, Box, BoxProfile, Event, Handler
from app.custody import apply_custody

router = APIRouter(prefix="/api")

def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

def rejudge(db, box_id):
    """Run the custody rules again on all events of one box."""
    events = db.scalars(select(Event).where(Event.box_id == box_id)).all()
    roles = {h.badge_id: h.role for h in db.scalars(select(Handler))}
    apply_custody(events, roles)

EventType = Literal["tap", "drop", "impact", "upside_down", "tamper", "temp_breach"]
Role = Literal["warehouse", "driver", "client"]

class EventIn(BaseModel):
    box_id: str | None = None
    seq: int
    uptime_ms: int
    type: EventType
    conf: float | None = None
    height_m: float | None = None
    temp_c: float | None = None
    hum_pct: float | None = None
    badge_id: str | None = None

TempMode = Literal["cold", "room", "medication"]

class ProfileIn(BaseModel):
    fragile: bool
    temp_mode: TempMode
    temp_min_c: float
    temp_max_c: float

class UploadIn(BaseModel):
    box_id: str
    uploaded_at_uptime_ms: int
    profile: ProfileIn | None = None  # optional: older tags do not send it
    events: list[EventIn]

class HandlerIn(BaseModel):
    badge_id: str
    name: str
    role: Role

@router.get("/health")
def health():
    return {"status": "ok"}

@router.post("/upload")
def upload(body: UploadIn, db: Session = Depends(get_db)):
    received = datetime.now(timezone.utc)

    if db.get(Box, body.box_id) is None:
        db.add(Box(box_id=body.box_id, first_seen=received))
        db.flush()

    if body.profile is not None:
        db.merge(BoxProfile(box_id=body.box_id, **body.profile.model_dump()))

    known = set(db.scalars(select(Event.seq).where(Event.box_id == body.box_id)))

    for e in sorted(body.events, key=lambda e: e.seq):
        if e.seq in known:
            continue
        known.add(e.seq)
        ago_ms = body.uploaded_at_uptime_ms - e.uptime_ms
        db.add(Event(
            box_id=body.box_id, seq=e.seq, uptime_ms=e.uptime_ms,
            event_time=received if ago_ms < 0 else received - timedelta(milliseconds=ago_ms),  # ago_ms < 0: event from before a reboot
            type=e.type, conf=e.conf, height_m=e.height_m,
            temp_c=e.temp_c, hum_pct=e.hum_pct, badge_id=e.badge_id,
        ))
    db.flush()
    rejudge(db, body.box_id)
    db.commit()

    ack = db.scalar(select(func.max(Event.seq)).where(Event.box_id == body.box_id))
    return {"ack_seq": ack or 0}

@router.get("/handlers")
def list_handlers(db: Session = Depends(get_db)):
    return [{"badge_id": h.badge_id, "name": h.name, "role": h.role}
            for h in db.scalars(select(Handler))]

@router.post("/handlers")
def add_handler(body: HandlerIn, db: Session = Depends(get_db)):
    db.merge(Handler(badge_id=body.badge_id, name=body.name, role=body.role))
    db.flush()
    for box_id in db.scalars(select(Box.box_id)):
        rejudge(db, box_id)
    db.commit()
    return body

@router.post("/reset")
def reset(db: Session = Depends(get_db)):
    db.execute(delete(Event))
    db.execute(delete(BoxProfile))
    db.execute(delete(Box))
    db.commit()
    return {"status": "reset"}


def iso(dt):
    """Time as ISO 8601 text, marked as UTC."""
    if dt is None:
        return None
    if dt.tzinfo is None:
        dt = dt.replace(tzinfo=timezone.utc)
    return dt.isoformat().replace("+00:00", "Z")

def person(badge_id, handlers):
    """Badge id to {badge_id, name, role}, or None if nobody."""
    if badge_id is None:
        return None
    h = handlers.get(badge_id)
    if h is None:
        return {"badge_id": badge_id, "name": "Unknown badge", "role": None}
    return {"badge_id": h.badge_id, "name": h.name, "role": h.role}

def profile_of(db, box_id):
    """The box's handling profile as a dict, or None if the tag never sent one."""
    p = db.get(BoxProfile, box_id)
    if p is None:
        return None
    return {"fragile": p.fragile, "temp_mode": p.temp_mode,
            "temp_min_c": p.temp_min_c, "temp_max_c": p.temp_max_c}

def box_status(events):
    severities = {e.severity for e in events if not e.expected}
    if "severe" in severities:
        return "severe"
    if "warning" in severities:
        return "warning"
    return "ok"

@router.get("/boxes")
def list_boxes(db: Session = Depends(get_db)):
    handlers = {h.badge_id: h for h in db.scalars(select(Handler))}
    result = []
    for box in db.scalars(select(Box).order_by(Box.box_id)):
        events = db.scalars(
            select(Event).where(Event.box_id == box.box_id).order_by(Event.seq)
        ).all()
        last = events[-1] if events else None
        result.append({
            "box_id": box.box_id,
            "status": box_status(events),
            "last_event_time": iso(last.event_time) if last else None,
            "current_holder": person(last.holder_badge_id, handlers) if last else None,
            "profile": profile_of(db, box.box_id),
        })
    return result


from fastapi import HTTPException
from app.custody import liable

@router.get("/boxes/{box_id}/timeline")
def timeline(box_id: str, db: Session = Depends(get_db)):
    if db.get(Box, box_id) is None:
        raise HTTPException(status_code=404, detail="Unknown box")

    handlers = {h.badge_id: h for h in db.scalars(select(Handler))}
    events = db.scalars(
        select(Event).where(Event.box_id == box_id).order_by(Event.seq)
    ).all()

    return {
        "box_id": box_id,
        "status": box_status(events),
        "profile": profile_of(db, box_id),
        "liable": [
            {"badge_id": l["badge_id"],
             "name": person(l["badge_id"], handlers)["name"],
             "reason": l["reason"]}
            for l in liable(events)
        ],
        "events": [
            {"seq": e.seq,
             "time": iso(e.event_time),
             "type": e.type,
             "severity": e.severity,
             "height_m": e.height_m,
             "temp_c": e.temp_c,
             "hum_pct": e.hum_pct,
             "holder": person(e.holder_badge_id, handlers),
             "expected": e.expected}
            for e in events
        ],
    }
