#!/bin/bash

# Sprawdź, czy podano poprawną liczbę argumentów
if [ "$#" -ne 1 ]; then
    echo "Użycie: $0 'tekst_do_przeszukania'"
    exit 1
fi

# Przechwyć argument
input_text="$1"

# Wyszukaj i wypisz ciągi zaczynające się od "/esp"
matches=$(echo $input_text | grep -o '/esp[^[:space:]]*')

# Katalog docelowy
destination_dir="/esp/flash"

# Kopiuj pliki do katalogu docelowego
for match in $matches; do
    # Kopiuj tylko pliki binarne
    if [[ $match == *.bin ]]; then
        cp -v "$match" "$destination_dir"
    fi
done

# Nazwa pliku tekstowego
output_file="output.txt"

# Utwórz plik tekstowy i wypisz zawartość input_text
echo "$input_text" > "$destination_dir/$output_file"

# Komunikat potwierdzający utworzenie pliku
echo "Utworzono plik $output_file z zawartością input_text w katalogu $destination_dir"

cp -vr /esp/ESP8266_RTOS_SDK/components/esptool_py "$destination_dir"
