import mockBoxes from './mock/boxes.json'
import mockTimeline from './mock/timeline.json'

const API_URL = import.meta.env.VITE_API_URL

export const USING_MOCK = !API_URL

async function getJson(path) {
  let res
  try {
    res = await fetch(`${API_URL}${path}`)
  } catch (err) {
    throw new Error(`Cannot reach the backend at ${API_URL} (${err.message})`)
  }
  if (!res.ok) {
    throw new Error(`Backend returned ${res.status} for ${path}`)
  }
  return res.json()
}

export async function getBoxes() {
  if (USING_MOCK) return mockBoxes
  return getJson('/api/boxes')
}

export async function getTimeline(boxId) {
  if (USING_MOCK) {
    return mockTimeline[boxId] ?? { box_id: boxId, status: 'ok', liable: [], events: [] }
  }
  return getJson(`/api/boxes/${boxId}/timeline`)
}
