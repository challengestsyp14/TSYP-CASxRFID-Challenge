import { EVENT_ICON, SEVERITY, ago, clock, eventTitle, personName, roleText } from '../labels'

function EventItem({ event, isNew }) {
  const sev = event.expected ? SEVERITY.info : (SEVERITY[event.severity] ?? SEVERITY.info)
  const role = roleText(event.holder)

  return (
    <li className="rise relative pl-14">
      <span className={`absolute left-0 top-2.5 flex h-10 w-10 items-center justify-center rounded-xl text-lg ring-1 ${sev.icon}`}>
        {EVENT_ICON[event.type] ?? '•'}
      </span>
      <div
        className={`rounded-2xl border p-4 transition-colors duration-700 ${
          isNew ? 'border-yellow-300/60 bg-yellow-300/15 shadow-lg shadow-yellow-500/10' : sev.card
        }`}
      >
        <div className="flex flex-wrap items-center justify-between gap-2">
          <div className="flex flex-wrap items-center gap-2">
            <span className="font-display font-semibold text-white">{eventTitle(event)}</span>
            {event.type !== 'tap' && (
              <span className={`rounded-full px-2 py-0.5 text-[11px] font-bold uppercase tracking-wide ${sev.pill}`}>
                {event.expected ? 'Expected' : sev.word}
              </span>
            )}
            {isNew && (
              <span className="rounded-full bg-yellow-300 px-2 py-0.5 text-[11px] font-bold uppercase tracking-wide text-yellow-950">
                New
              </span>
            )}
          </div>
          <span className="font-mono text-xs text-slate-400" title={new Date(event.time).toLocaleString()}>
            {clock(event.time)} · {ago(event.time)}
          </span>
        </div>

        <div className="mt-1.5 flex flex-wrap gap-x-4 gap-y-1 text-sm text-slate-400">
          {event.type !== 'tap' ? (
            <span>
              Held by <span className="font-medium text-slate-200">{event.holder ? personName(event.holder) : 'nobody yet'}</span>
              {role && ` · ${role}`}
            </span>
          ) : (
            role && <span>{role}</span>
          )}
          {event.temp_c != null && (
            <span className={`font-mono ${event.type === 'temp_breach' ? 'font-semibold text-rose-300' : ''}`}>
              {event.temp_c} °C{event.hum_pct != null && ` · ${event.hum_pct} % RH`}
            </span>
          )}
        </div>
      </div>
    </li>
  )
}

export default EventItem
