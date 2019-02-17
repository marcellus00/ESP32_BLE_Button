import pygatt

adapter = pygatt.GATTToolBackend()

def handle_data(handle, value):
    print("Button is ON" if value[0] == 1 else "Button is OFF")

try:
    print("Reading")
    adapter.start()
    device = adapter.connect('24:0a:c4:ae:89:3a')
    device.subscribe("12cd634c-2911-45c5-8c46-3d67628a88fd", callback=handle_data)
    input("Press enter to stop program...\n")
finally:
    print("Done")
    adapter.stop()
