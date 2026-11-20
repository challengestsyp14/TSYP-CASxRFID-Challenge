from datetime import datetime
from sqlalchemy import create_engine, ForeignKey, UniqueConstraint
from sqlalchemy.orm import DeclarativeBase, Mapped, mapped_column, sessionmaker

engine = create_engine(
    "sqlite:///./parcel.db",
    connect_args={"check_same_thread": False},
)
SessionLocal = sessionmaker(bind=engine, autoflush=False)

class Base(DeclarativeBase):
    pass

class Box(Base):
    __tablename__ = "boxes"
    box_id: Mapped[str] = mapped_column(primary_key=True)
    first_seen: Mapped[datetime]

class BoxProfile(Base):
    """The handling profile chosen on the tag (latest upload wins)."""
    __tablename__ = "box_profiles"
    box_id: Mapped[str] = mapped_column(ForeignKey("boxes.box_id"), primary_key=True)
    fragile: Mapped[bool]
    temp_mode: Mapped[str]  # cold, room or medication
    temp_min_c: Mapped[float]
    temp_max_c: Mapped[float]

class Handler(Base):
    __tablename__ = "handlers"
    badge_id: Mapped[str] = mapped_column(primary_key=True)
    name: Mapped[str]
    role: Mapped[str]  # warehouse, driver or client

class Event(Base):
    __tablename__ = "events"
    __table_args__ = (UniqueConstraint("box_id", "seq", name="uq_box_seq"),)

    id: Mapped[int] = mapped_column(primary_key=True, autoincrement=True)
    box_id: Mapped[str] = mapped_column(ForeignKey("boxes.box_id"))
    seq: Mapped[int]
    uptime_ms: Mapped[int]
    event_time: Mapped[datetime | None]
    type: Mapped[str]  # tap, drop, impact, upside_down, tamper, temp_breach
    conf: Mapped[float | None]
    height_m: Mapped[float | None]
    temp_c: Mapped[float | None]
    hum_pct: Mapped[float | None]
    badge_id: Mapped[str | None]
    holder_badge_id: Mapped[str | None]
    severity: Mapped[str | None]  # info, warning or severe
    expected: Mapped[bool] = mapped_column(default=False)
