@echo off
setlocal

rem Pobieranie aktualnej lokalizacji (bieżącego katalogu)
set "local_path=%cd%"

rem Wyświetlanie aktualnej lokalizacji
echo Aktualna lokalizacja: %local_path%

rem Dołączanie ścieżki do Docker run
docker run -it --rm -v %local_path%:/esp/flash esp_env:latest

rem Pauza, aby zatrzymać okno
pause


@REM docker run -it --rm -v $(PWD):/backup nazwa_obrazu:tag
@REM pause