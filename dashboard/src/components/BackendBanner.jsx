function BackendBanner({ error }) {
  return (
    <div className="mb-6 flex items-start gap-3 rounded-2xl border border-rose-400/30 bg-rose-500/10 p-4">
      <span className="text-xl">🔌</span>
      <div>
        <p className="font-display font-semibold text-rose-200">Cannot reach the backend</p>
        <p className="text-sm text-rose-300/80">
          Is the backend window still running? Showing the last data received. ({error})
        </p>
      </div>
    </div>
  )
}

export default BackendBanner
