import serial

acknowledge = 'K'
fail = 'F'
endCmd = '$'

ser = serial.Serial(
    port='COM10',      # anpassen
    baudrate=9600,
    timeout=1
)

def coordsToGcode(coords):
    Gcodes = ['M3'+endCmd]
    for x, y in coords:                      # Unterscheidung zwischen G0 und M Befehlen fehlt!!!
        Gcodes.append(f'G0 X{x} Y{y}{endCmd}')
    Gcodes.append('M5' + endCmd)
    return Gcodes

#commands = ['M3'+endCmd, 'G0 X100 Y100'+endCmd, 'G0 X200 Y0'+endCmd, 'G0 X100 Y-100'+endCmd, 'G0 X0 Y0'+endCmd, 'M5'+endCmd]
commands = coordsToGcode([
            (100,100),
			(200,0),
			(100,-100),
			(0,0),
			(0,100),
			(200,100),
			(200,-100),
			(0,-100),
			(0,0)])  # Beispielkoordinaten
cmd_index = 0

while cmd_index < len(commands):
    rx = ser.read(1) # read one byte

    if rx.decode() == fail:
        ser.write(commands[cmd_index].encode()) # neu senden
    if rx.decode() == acknowledge:
        ser.write(commands[cmd_index].encode())
        print("TX:", commands[cmd_index])
        cmd_index += 1
        
print("All commands sent.")
ser.close()