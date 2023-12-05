import argparse
import re

def modify_text(input_text, port_value):
    # Zastosuj regex do zamiany ścieżek na nazwy plików
    modified_text = re.sub(r'/esp[^ \n]*\.bin', lambda x: x.group().split('/')[-1], input_text)

    # Zastąp określony ciąg znaków
    modified_text = modified_text.replace("/esp/ESP8266_RTOS_SDK/components", ".")

    # Zastąp konkretne miejsce w tekście wartością portu
    modified_text = modified_text.replace("/dev/ttyUSB0", port_value)

    return modified_text

def main():
    parser = argparse.ArgumentParser(description='Modify text and save it to a file.')
    parser.add_argument('port', help='Port value to replace /dev/ttyUSB0 in the text')

    args = parser.parse_args()

    input_file_path = "output.txt"
    output_file_path = "flash.bat"

    # Otwórz plik wejściowy
    with open(input_file_path, 'r') as input_file:
        # Odczytaj zawartość pliku
        input_text = input_file.read()

    # Modyfikuj tekst
    modified_text = modify_text(input_text, args.port)

    # Otwórz plik wyjściowy i zapisz zmodyfikowaną zawartość
    with open(output_file_path, 'w') as output_file:
        output_file.write(modified_text)

    print(f"Utworzono plik {output_file_path} z zmodyfikowaną zawartością.")

if __name__ == "__main__":
    main()
