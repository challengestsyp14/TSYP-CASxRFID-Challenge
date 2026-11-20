from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.models import Base, engine, SessionLocal, Handler
from app.routes import router

# Real demo badges read from the tag
DEMO_HANDLERS = [
    ("17733800", "Warehouse A", "warehouse"),
    ("4BBEF804", "Driver B", "driver"),
    ("53A0B3A7", "Client C", "client"),
]

app = FastAPI(title="Parcel Custody Tag")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

Base.metadata.create_all(bind=engine)

with SessionLocal() as db:
    for badge_id, name, role in DEMO_HANDLERS:
        db.merge(Handler(badge_id=badge_id, name=name, role=role))
    db.commit()

app.include_router(router)
