// The handling profile chosen on the tag: fragile or not, and the temperature mode with its alert limits
const MODE_NAMES = {
  cold: 'Cold chain',
  room: 'Room temperature',
  medication: 'Medication',
}

function ProfileBadge({ profile, size = 'md' }) {
  const text = size === 'sm' ? 'text-xs' : 'text-sm'

  if (!profile) {
    return <span className={`${text} text-slate-500`}>Handling profile not received yet</span>
  }

  return (
    <div className="flex flex-wrap gap-2">
      <span
        title={profile.fragile ? 'Every drop and every hit is an alert' : 'Only drops of 0.30 m or more are alerts'}
        className={`inline-flex items-center gap-1.5 rounded-full px-3 py-1 font-semibold ring-1 ${text} ${
          profile.fragile
            ? 'bg-fuchsia-500/10 text-fuchsia-300 ring-fuchsia-400/30'
            : 'bg-white/5 text-slate-300 ring-white/10'
        }`}
      >
        {profile.fragile ? '⚠ Fragile' : 'Not fragile'}
      </span>
      <span
        title={`An alert is raised below ${profile.temp_min_c} °C or above ${profile.temp_max_c} °C`}
        className={`inline-flex items-center gap-1.5 rounded-full bg-sky-400/10 px-3 py-1 font-semibold text-sky-300 ring-1 ring-sky-400/30 ${text}`}
      >
        🌡 {MODE_NAMES[profile.temp_mode] ?? profile.temp_mode}
        <span className="font-mono font-normal text-sky-200/80">
          {profile.temp_min_c}–{profile.temp_max_c} °C
        </span>
      </span>
    </div>
  )
}

export default ProfileBadge
