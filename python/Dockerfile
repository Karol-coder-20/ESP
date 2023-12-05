# Użyj obrazu Pythona 3.8 jako bazowego obrazu
FROM python:3.8

# Skopiuj kod źródłowy do kontenera
COPY . /app

# Ustaw bieżący katalog jako katalog roboczy
WORKDIR /app

# Zainstaluj zależności
RUN pip install --upgrade pip

# Uruchom serwer po starcie kontenera
CMD [ "python", "ota.py" ]
