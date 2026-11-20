// Plain-language words and colours for everything the backend sends

export const STATUS = {
  ok: {
    word: 'All good',
    dot: 'bg-emerald-400',
    pill: 'bg-emerald-400/10 text-emerald-300 ring-emerald-400/30',
    bar: 'from-emerald-400 to-teal-400',
    glow: 'hover:shadow-emerald-500/10',
  },
  warning: {
    word: 'Warning',
    dot: 'bg-amber-400',
    pill: 'bg-amber-400/10 text-amber-300 ring-amber-400/30',
    bar: 'from-amber-400 to-orange-400',
    glow: 'hover:shadow-amber-500/15',
  },
  severe: {
    word: 'Serious problem',
    dot: 'bg-rose-500',
    pill: 'bg-rose-500/10 text-rose-300 ring-rose-400/30',
    bar: 'from-rose-500 to-fuchsia-500',
    glow: 'hover:shadow-rose-500/20',
  },
}

export const SEVERITY = {
  info: {
    word: 'Info',
    pill: 'bg-white/10 text-slate-300',
    card: 'border-white/10 bg-white/[0.03]',
    icon: 'bg-slate-800 ring-white/10',
  },
  warning: {
    word: 'Warning',
    pill: 'bg-amber-400/15 text-amber-300',
    card: 'border-amber-400/25 bg-amber-400/[0.06]',
    icon: 'bg-amber-950 ring-amber-400/40',
  },
  severe: {
    word: 'Serious',
    pill: 'bg-rose-500/15 text-rose-300',
    card: 'border-rose-400/30 bg-rose-500/[0.07]',
    icon: 'bg-rose-950 ring-rose-400/50',
  },
}

export const ROLE = {
  warehouse: 'Warehouse',
  driver: 'Driver',
  client: 'Client',
}

export const EVENT_ICON = {
  tap: '🪪',
  drop: '⬇️',
  impact: '💥',
  upside_down: '🔄',
  tamper: '📦',
  temp_breach: '🌡️',
}

// One sentence per event
export function eventTitle(e) {
  switch (e.type) {
    case 'tap':
      return e.holder ? `${personName(e.holder)} took the box` : 'Badge tapped'
    case 'drop':
      return e.height_m != null ? `Dropped from about ${e.height_m} m` : 'Dropped'
    case 'impact':
      return 'Hit hard (impact)'
    case 'upside_down':
      return 'Turned upside down'
    case 'tamper':
      return e.expected ? 'Opened by the client (expected)' : 'Lid opened'
    case 'temp_breach':
      return e.temp_c != null ? `Temperature out of range: ${e.temp_c} °C` : 'Temperature out of range'
    default:
      return e.type
  }
}

// The backend's short reasons ("drop 0.8 m", "tamper", "temp_breach") in words
export function reasonText(reason) {
  if (!reason) return ''
  if (reason.startsWith('drop')) return `a drop${reason.slice(4) ? ` of ${reason.slice(5)}` : ''}`
  const words = {
    impact: 'a hard hit',
    upside_down: 'turning it upside down',
    tamper: 'opening the lid',
    temp_breach: 'a temperature problem',
  }
  return words[reason] ?? reason
}

export function personName(p) {
  if (!p) return 'Nobody'
  return p.name || 'Unknown badge'
}

export function roleText(p) {
  if (!p || !p.role) return null
  return ROLE[p.role] ?? p.role
}

// "2 min ago", "just now"
export function ago(iso, now = Date.now()) {
  if (!iso) return null
  const s = Math.max(0, Math.round((now - new Date(iso).getTime()) / 1000))
  if (s < 10) return 'just now'
  if (s < 60) return `${s} s ago`
  const m = Math.round(s / 60)
  if (m < 60) return `${m} min ago`
  const h = Math.round(m / 60)
  if (h < 24) return `${h} h ago`
  return new Date(iso).toLocaleDateString()
}

export function clock(iso) {
  if (!iso) return ''
  return new Date(iso).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' })
}
