"""Custody and severity rules from the Contracts. No database access."""

DROP_SEVERE_M = 0.30          # to be tuned on the hardware
SAME_TAP_IGNORE_S = 2.0

def severity_of(e):
    if e.type == "tap":
        return "info"
    if e.type == "drop":
        return "severe" if (e.height_m or 0) >= DROP_SEVERE_M else "warning"
    if e.type == "impact":
        return "severe"
    if e.type == "tamper":
        return "info" if e.expected else "severe"
    return "warning"  # upside_down, temp_breach

def apply_custody(events, roles):
    """Fill holder_badge_id, expected and severity on each event.
    events: objects with seq, event_time, type, height_m, badge_id.
    roles: {badge_id: role}.
    Returns (events sorted by seq, list of handovers as (seq, badge_id))."""
    events = sorted(events, key=lambda e: e.seq)
    holder = None
    last_tap = None          # (badge_id, event_time) of the last accepted tap
    handovers = []

    for e in events:
        if e.type == "tap":
            same_badge_soon = (
                last_tap is not None
                and last_tap[0] == e.badge_id
                and e.event_time is not None
                and last_tap[1] is not None
                and abs((e.event_time - last_tap[1]).total_seconds()) < SAME_TAP_IGNORE_S
            )
            if not same_badge_soon:
                holder = e.badge_id
                last_tap = (e.badge_id, e.event_time)
                handovers.append((e.seq, e.badge_id))
            e.holder_badge_id = holder
            e.expected = False
        else:
            e.holder_badge_id = holder
            e.expected = e.type == "tamper" and roles.get(holder) == "client"
        e.severity = severity_of(e)

    return events, handovers

def liable(events):
    """Holders of every non-expected warning or severe event, with reasons."""
    reasons = {}
    for e in events:
        if e.severity in ("warning", "severe") and not e.expected and e.holder_badge_id:
            text = f"drop {e.height_m} m" if e.type == "drop" and e.height_m else e.type
            reasons.setdefault(e.holder_badge_id, []).append(text)
    return [{"badge_id": b, "reason": ", ".join(r)} for b, r in reasons.items()]
