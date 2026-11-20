import { useState } from 'react'
import { USING_MOCK } from './api'
import BoxDetail from './components/BoxDetail'
import BoxList from './components/BoxList'

function App() {
  const [selectedBoxId, setSelectedBoxId] = useState(null)

  return (
    <div className="min-h-screen">
      <header className="sticky top-0 z-10 border-b border-white/10 bg-slate-950/60 backdrop-blur-xl">
        <div className="mx-auto flex max-w-6xl items-center justify-between gap-4 px-6 py-4">
          <button type="button" onClick={() => setSelectedBoxId(null)} className="flex items-center gap-3 text-left">
            <span className="flex h-10 w-10 items-center justify-center rounded-xl bg-gradient-to-br from-sky-400 to-violet-500 text-xl shadow-lg shadow-sky-500/30">📦</span>
            <span>
              <span className="block font-display text-xl font-bold tracking-tight text-white">Parcel Custody</span>
              <span className="block text-xs text-slate-400">Who held each box, and what happened to it</span>
            </span>
          </button>
          <span className="inline-flex items-center gap-2 rounded-full border border-emerald-400/30 bg-emerald-400/10 px-3 py-1 text-xs font-semibold text-emerald-300">
            <span className="relative flex h-2 w-2">
              <span className="absolute inline-flex h-full w-full animate-ping rounded-full bg-emerald-400 opacity-75" />
              <span className="relative inline-flex h-2 w-2 rounded-full bg-emerald-400" />
            </span>
            LIVE
          </span>
        </div>
      </header>
      <main className="mx-auto max-w-6xl px-6 py-8">
        {selectedBoxId ? (
          <BoxDetail key={selectedBoxId} boxId={selectedBoxId} onBack={() => setSelectedBoxId(null)} />
        ) : (
          <BoxList onSelect={setSelectedBoxId} />
        )}
      </main>
      {USING_MOCK && (
        <div className="fixed right-4 bottom-4 rounded-lg bg-purple-700 px-3 py-1 text-sm font-bold tracking-wide text-white shadow">
          MOCK DATA
        </div>
      )}
    </div>
  )
}

export default App
