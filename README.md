# The code for the panel P10 + ESP32
*********
* List of features
    + Edit text in real time via HomeAssistiant
    + Split sentences into lines (features)
    + Enable/disable text scrolling (features)
    + Add a drawing as a binary image (features)
    + State of panel
    + Remote panel on/off
# Docs for API
* Endpoints
    + 10.131.170.4 - the local static IP
    + /api/text - endpoint for editing text (POST)
    + /api/led - endpint for on/off panel (POST)
    + /api/led - endpoint for check on/off panel (GET), return "{"led":"true"}" if panel ON and "{"led":"false"}" if panel OFF
