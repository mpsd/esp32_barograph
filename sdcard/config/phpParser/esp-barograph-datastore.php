<?php

/*
struct datasetstruct {
    float_t lat;
    float_t lon;
    float_t altitude_m;
    float_t course;
    float_t speed;
    uint32_t sat;
    float_t hdop;
    float_t temperature_raw;
    float_t temperature;
    float_t temperature_offset;
    float_t humidity_raw;
    float_t humidity;
    float_t pressure_raw;
    float_t pressure;
    float_t altitude;
    uint64_t timestamp;
};

f	Gleitkommazahl (maschinenabhängige Größe und Wiedergabe)
g	Gleitkommazahl (maschinenabhängige Größe, Byte-Folge Little Endian)
G	Gleitkommazahl (maschinenabhängige Größe, Byte-Folge Big Endian)

l	vorzeichenbehafteter Long-Typ (immer 32 Bit, Byte-Folge maschinenabhängig)
L	vorzeichenloser Long-Typ (immer 32 Bit, Byte-Folge maschinenabhängig)
N	vorzeichenloser Long-Typ (immer 32 Bit, Byte-Folge Big Endian)
V	vorzeichenloser Long-Typ (immer 32 Bit, Byte-Folge Little Endian)

q	vorzeichenbehafteter Long-Long-Typ (immer 64 bit, maschinenabhängig)
Q	vorzeichenloser Long-Long-Typ (immer 64 bit, maschinenabhängig)
J	vorzeichenloser Long-Long-Typ (immer 64 bit, Byte-Folge Big Endian)
P	vorzeichenloser Long-Long-Typ (immer 64 bit, Byte-Folge Little Endian)


Laenge der Datentypen beim ESP8266

sizeof void :        1 byte
sizeof bool :        1 byte
sizeof char :        1 byte
sizeof byte :        1 byte
sizeof uint8_t :     1 byte

sizeof short :       2 byte
sizeof uint16_t :    2 byte

sizeof word :        4 byte
sizeof int :         4 byte
sizeof long :        4 byte
sizeof float :       4 byte
sizeof uint32_t :    4 byte

sizeof double :      8 byte
sizeof long long :   8 byte
sizeof uint64_t :    8 byte

due to whatever reason the uint64_t timestamp is coded not as 8byte long but as two 4Byte

*/

$format = "glat/glon/galtitude_m/gcourse/gspeed/Vsat/ghdop/gtemperature_raw/gtemperature/gtemperature_offset/ghumidity_raw/g_humidity/gpressure_raw/gpressure/galtitude/Ltstlow/Ltimestamp/C4eos";

$filesize = filesize("datastore"); // path to datastore file
$fp = fopen("datastore", "rb");
$binary = fread($fp, $filesize);
fclose($fp);

// this variable holds the size of *one* structure
$block_size = (14 * 4) + 4 + 8 + 4;
$num_blocks = $filesize/$block_size;
// $num_blocks=3;

date_default_timezone_set("UTC");

// extract each block in a loop from the binary string
for ($i = 0, $offset = 0; $i < $num_blocks; $i++, $offset += $block_size) {
   $unpacked_block[$i] = unpack($format, substr($binary, $offset));
   $unpacked_block[$i]['time'] = date("Y-m-d H:i:s", $unpacked_block[$i]['timestamp']);  
}

var_dump( $unpacked_block );

/* alternatively output to CSV file

$fp = fopen("out.csv","w");
fputcsv($fp, array_keys(current($unpacked_block)), ";");

// write out the data
foreach ( $unpacked_block as $row ) {
           fputcsv($fp, $row, ";");
}
fclose($fp);

*/