import { useEffect, useState } from 'react'
import { getBoxes } from '../api'
import { STATUS, ago, personName, roleText } from '../labels'
import BackendBanner from './BackendBanner'
import ProfileBadge from './ProfileBadge'

const POLL_MS = 2000

function Summary({ boxes }) {
  const count = (s) => boxes.filter((b) => b.status === s).length
  const items = [
    { label: 'Boxes tracked', value: boxes.length, cls: 'text-white', ring: 'from-sky-400/40' },
    { label: 'All good', value: count('ok'), cls: 'text-emerald-300', ring: 'from-emerald-400/40' },
    { label: 'Warning', value: count('warning'), cls: 'text-amber-300', ring: 'from-amber-400/40' },
    { label: 'Serious problem', value: count('severe'), cls: 'text-rose-300', ring: 'from-rose-500/50' },
  ]
  return (
    <div className="mb-8 grid grid-cols-2 gap-4 sm:grid-cols-4">
      {items.map((i) => (
        <div key={i.label} className={`rounded-2xl bg-gradient-to-br ${i.ring} to-transparent p-px`}>
          <div className="h-full rounded-2xl bg-slate-950/80 p-4 backdrop-blur">
            <div className={`font-display text-4xl font-bold ${i.cls}`}>{i.value}</div>
            <div className="mt-1 text-sm text-slate-400">{i.label}</div>
          </div>
        </div>
      ))}
    </div>
  )
}

function BoxList({ onSelect }) {
  const [boxes, setBoxes] = useState(null)
  const [error, setError] = useState(null)

  useEffect(() => {
    let cancelled = false

    async function load() {
      try {
        const data = await getBoxes()
        if (cancelled) return
        setBoxes(data)
        setError(null)
      } catch (err) {
        if (!cancelled) setError(err.message)
      }
    }

    load()
    const interval = setInterval(load, POLL_MS)
    return () => {
      cancelled = true
      clearInterval(interval)
    }
  }, [])

  return (
    <div>
      {error && <BackendBanner error={error} />}

      {!boxes ? (
        !error && <p className="text-slate-400">Loading…</p>
      ) : boxes.length === 0 ? (
        <div className="rounded-3xl border border-dashed border-white/15 bg-white/[0.03] p-14 text-center backdrop-blur">
          <div className="mb-4 text-6xl">📦</div>
          <p className="font-display text-2xl font-semibold text-white">No boxes yet</p>
          <p className="mt-2 text-slate-400">Switch on a tag. Its box appears here after its first upload.</p>
        </div>
      ) : (
        <>
          <Summary boxes={boxes} />
          <div className="grid grid-cols-1 gap-5 sm:grid-cols-2 lg:grid-cols-3">
            {boxes.map((box, idx) => {
              const st = STATUS[box.status] ?? STATUS.ok
              const role = roleText(box.current_holder)
              return (
                <button
                  key={box.box_id}
                  type="button"
                  onClick={() => onSelect(box.box_id)}
                  style={{ animationDelay: `${idx * 60}ms` }}
                  className={`rise group relative flex flex-col gap-5 overflow-hidden rounded-3xl border border-white/10 bg-white/[0.04] p-6 text-left shadow-xl shadow-black/30 backdrop-blur transition duration-300 hover:-translate-y-1 hover:border-white/20 hover:bg-white/[0.07] hover:shadow-2xl ${st.glow}`}
                >
                  <span className={`absolute inset-x-0 top-0 h-1 bg-gradient-to-r ${st.bar}`} />
                  <div className="flex items-center justify-between gap-3">
                    <div className="flex items-center gap-3">
                      <span className="flex h-11 w-11 items-center justify-center rounded-2xl bg-white/5 text-2xl ring-1 ring-white/10">📦</span>
                      <span className="font-display text-2xl font-bold tracking-tight text-white">{box.box_id}</span>
                    </div>
                    <span className={`inline-flex items-center gap-1.5 rounded-full px-3 py-1 text-xs font-bold ring-1 ${st.pill}`}>
                      <span className={`h-1.5 w-1.5 rounded-full ${st.dot}`} />
                      {st.word}
                    </span>
                  </div>

                  <div>
                    <div className="text-[11px] font-semibold uppercase tracking-[0.15em] text-slate-500">Now held by</div>
                    <div className="mt-0.5 text-lg font-medium text-slate-100">
                      {box.current_holder ? personName(box.current_holder) : 'Nobody yet'}
                      {role && <span className="font-normal text-slate-400"> · {role}</span>}
                    </div>
                  </div>

                  <ProfileBadge profile={box.profile} size="sm" />

                  <div className="mt-auto flex items-center justify-between border-t border-white/10 pt-4 text-sm">
                    <span className="text-slate-400">
                      {box.last_event_time ? `Last activity ${ago(box.last_event_time)}` : 'No events yet'}
                    </span>
                    <span className="font-semibold text-sky-300 transition group-hover:translate-x-1">Timeline →</span>
                  </div>
                </button>
              )
            })}
          </div>
        </>
      )}
    </div>
  )
}

export default BoxList
