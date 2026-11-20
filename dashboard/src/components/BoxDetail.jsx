import { useEffect, useRef, useState } from 'react'
import { getTimeline } from '../api'
import BackendBanner from './BackendBanner'
import EventItem from './EventItem'
import LiabilityCard from './LiabilityCard'
import ProfileBadge from './ProfileBadge'
import { STATUS, clock, personName, roleText } from '../labels'

const POLL_MS = 2000
const HIGHLIGHT_MS = 3000

function BoxDetail({ boxId, onBack }) {
  const [timeline, setTimeline] = useState(null)
  const [error, setError] = useState(null)
  const [newSeqs, setNewSeqs] = useState(new Set())
  // seqs in the last response; null until the first load so existing events are not highlighted
  const seenSeqs = useRef(null)

  useEffect(() => {
    let cancelled = false
    const timers = new Set()
    seenSeqs.current = null

    async function load() {
      try {
        const data = await getTimeline(boxId)
        if (cancelled) return

        const seqs = data.events.map((e) => e.seq)
        if (seenSeqs.current) {
          const fresh = seqs.filter((s) => !seenSeqs.current.has(s))
          if (fresh.length > 0) {
            setNewSeqs((cur) => new Set([...cur, ...fresh]))
            const timer = setTimeout(() => {
              timers.delete(timer)
              setNewSeqs((cur) => {
                const next = new Set(cur)
                fresh.forEach((s) => next.delete(s))
                return next
              })
            }, HIGHLIGHT_MS)
            timers.add(timer)
          }
        }
        seenSeqs.current = new Set(seqs)

        setTimeline(data)
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
      timers.forEach(clearTimeout)
    }
  }, [boxId])

  return (
    <div>
      <button
        type="button"
        onClick={onBack}
        className="mb-6 inline-flex items-center gap-2 rounded-full border border-white/10 bg-white/5 px-4 py-1.5 text-sm font-medium text-slate-200 transition hover:bg-white/10"
      >
        &larr; All boxes
      </button>

      {error && <BackendBanner error={error} />}

      {!timeline ? (
        !error && <p className="text-slate-400">Loading…</p>
      ) : (
        <BoxView timeline={timeline} newSeqs={newSeqs} />
      )}
    </div>
  )
}

const PANEL = 'rounded-3xl border border-white/10 bg-white/[0.04] p-6 shadow-xl shadow-black/30 backdrop-blur'
const LABEL = 'text-[11px] font-semibold uppercase tracking-[0.15em] text-slate-500'

// The handovers, in order: Warehouse A → Driver B → Client C
function CustodyChain({ events }) {
  const taps = events.filter((e) => e.type === 'tap')
  if (taps.length === 0) {
    return <p className="text-sm text-slate-400">Nobody has tapped a badge on this box yet.</p>
  }
  return (
    <ol className="flex flex-wrap items-center gap-2">
      {taps.map((t, i) => {
        const current = i === taps.length - 1
        return (
          <li key={t.seq} className="flex items-center gap-2">
            {i > 0 && <span className="text-lg text-slate-600">⟶</span>}
            <span
              className={`rounded-2xl border px-4 py-2.5 ${
                current ? 'border-sky-400/40 bg-sky-400/10 shadow-lg shadow-sky-500/10' : 'border-white/10 bg-white/[0.03]'
              }`}
            >
              <span className="font-display font-semibold text-white">{personName(t.holder)}</span>
              {roleText(t.holder) && <span className="text-sm text-slate-400"> · {roleText(t.holder)}</span>}
              <span className="block font-mono text-[11px] text-slate-500">
                {current ? 'holds it since ' : 'from '}
                {clock(t.time)}
              </span>
            </span>
          </li>
        )
      })}
    </ol>
  )
}

function BoxView({ timeline, newSeqs }) {
  const st = STATUS[timeline.status] ?? STATUS.ok
  const events = [...timeline.events].sort((a, b) => a.seq - b.seq)
  const lastHolder = events.length ? events[events.length - 1].holder : null

  return (
    <div className="flex flex-col gap-6">
      <div className={`${PANEL} relative overflow-hidden`}>
        <span className={`absolute inset-x-0 top-0 h-1 bg-gradient-to-r ${st.bar}`} />
        <div className="flex flex-wrap items-center justify-between gap-4">
          <div className="flex items-center gap-4">
            <span className="flex h-14 w-14 items-center justify-center rounded-2xl bg-white/5 text-3xl ring-1 ring-white/10">📦</span>
            <div>
              <div className={LABEL}>Parcel</div>
              <h1 className="font-display text-4xl font-bold tracking-tight text-white">{timeline.box_id}</h1>
            </div>
          </div>
          <span className={`inline-flex items-center gap-2 rounded-full px-4 py-2 text-sm font-bold ring-1 ${st.pill}`}>
            <span className={`h-2 w-2 rounded-full ${st.dot}`} />
            {st.word}
          </span>
        </div>
        <div className="mt-6 grid gap-5 sm:grid-cols-2">
          <div>
            <div className={LABEL}>Now held by</div>
            <div className="mt-1 text-xl font-medium text-slate-100">
              {lastHolder ? personName(lastHolder) : 'Nobody yet'}
              {roleText(lastHolder) && <span className="font-normal text-slate-400"> · {roleText(lastHolder)}</span>}
            </div>
          </div>
          <div>
            <div className={`${LABEL} mb-2`}>Handling profile</div>
            <ProfileBadge profile={timeline.profile} size="sm" />
          </div>
        </div>
      </div>

      <LiabilityCard liable={timeline.liable} />

      <section className={PANEL}>
        <h2 className="mb-4 font-display text-lg font-semibold text-white">Chain of custody</h2>
        <CustodyChain events={events} />
      </section>

      <section>
        <h2 className="mb-4 flex items-baseline justify-between font-display text-lg font-semibold text-white">
          What happened
          <span className="font-sans text-xs font-normal uppercase tracking-[0.15em] text-slate-500">newest first</span>
        </h2>
        {events.length === 0 ? (
          <p className="text-slate-400">No events yet.</p>
        ) : (
          <ol className="relative flex flex-col gap-3 before:absolute before:bottom-4 before:left-[19px] before:top-4 before:w-px before:bg-gradient-to-b before:from-white/20 before:to-transparent">
            {[...events].reverse().map((event) => (
              <EventItem key={event.seq} event={event} isNew={newSeqs.has(event.seq)} />
            ))}
          </ol>
        )}
      </section>
    </div>
  )
}

export default BoxDetail
