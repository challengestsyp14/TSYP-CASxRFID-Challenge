import { reasonText } from '../labels'

// Who is responsible, grouped by person: "Driver B · 3 problems: a drop of 0.8 m, opening the lid, ..."
function LiabilityCard({ liable }) {
  if (liable.length === 0) {
    return (
      <div className="flex items-center gap-4 rounded-2xl border border-emerald-400/25 bg-emerald-400/[0.07] p-5">
        <span className="flex h-11 w-11 items-center justify-center rounded-xl bg-emerald-400/15 text-xl">✓</span>
        <div>
          <h2 className="font-display text-lg font-semibold text-emerald-200">Nobody is responsible for damage</h2>
          <p className="text-sm text-emerald-300/70">No problem was recorded while anyone held this box.</p>
        </div>
      </div>
    )
  }

  const people = []
  for (const entry of liable) {
    let p = people.find((x) => x.badge_id === entry.badge_id)
    if (!p) {
      p = { badge_id: entry.badge_id, name: entry.name || 'Unknown badge', reasons: [] }
      people.push(p)
    }
    // the backend joins one person's problems with commas: "drop 0.8 m, tamper, temp_breach"
    for (const r of (entry.reason || '').split(',')) {
      if (r.trim()) p.reasons.push(reasonText(r.trim()))
    }
  }

  return (
    <div className="relative overflow-hidden rounded-2xl border border-rose-400/30 bg-gradient-to-br from-rose-500/15 via-rose-500/[0.06] to-transparent p-5">
      <div className="mb-4 flex items-center gap-3">
        <span className="flex h-11 w-11 items-center justify-center rounded-xl bg-rose-500/20 text-xl">⚖️</span>
        <div>
          <h2 className="font-display text-lg font-semibold text-rose-100">Responsible</h2>
          <p className="text-sm text-rose-200/70">Who was holding the box when each problem happened</p>
        </div>
      </div>
      <ul className="flex flex-col gap-2">
        {people.map((p) => (
          <li key={p.badge_id} className="flex flex-wrap items-center gap-x-3 gap-y-2 rounded-xl border border-white/10 bg-black/20 px-4 py-3">
            <span className="font-display text-base font-semibold text-white">{p.name}</span>
            <span className="rounded-full bg-rose-500/20 px-2.5 py-0.5 text-xs font-semibold text-rose-200">
              {p.reasons.length} problem{p.reasons.length > 1 ? 's' : ''}
            </span>
            <span className="text-sm text-slate-300">{p.reasons.join(' · ')}</span>
          </li>
        ))}
      </ul>
    </div>
  )
}

export default LiabilityCard
