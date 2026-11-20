# Deletes all events so the demo can restart
Invoke-RestMethod -Method Post -Uri http://localhost:8000/api/reset
