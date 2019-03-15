if exist ffmpeg\Build\Windows10\x86\nul goto end

curl -L -o temp.zip "https://onedrive.live.com/download?cid=8DFA3705CA6CB7CC&resid=8DFA3705CA6CB7CC%%21128910&authkey=AI1twiJV1hLTTiw" 
"C:\Program Files\7-zip\7z" x temp.zip

:end