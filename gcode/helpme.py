import serial

print(serial)
print(type(serial))
print(getattr(serial, "__file__", "keine Datei"))
print(dir(serial))

print(serial.__path__)