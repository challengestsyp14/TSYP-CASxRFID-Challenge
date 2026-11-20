# Starts the backend and the dashboard, each in its own window
$root = $PSScriptRoot

Start-Process powershell -ArgumentList "-NoExit", "-Command", "cd '$root\backend'; .\venv\Scripts\activate; uvicorn app.main:app --host 0.0.0.0 --port 8000"

Start-Process powershell -ArgumentList "-NoExit", "-Command", "cd '$root\dashboard'; `$env:VITE_API_URL='http://localhost:8000'; npm run dev"
