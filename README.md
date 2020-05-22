# MediaPlayer
The project aims to make a media player which plays the ringtones (WAV Files) using the C++ application and control it using the IOWarrior 40 hardware kit. Users can control the start/stop/resume operation using the button on IOWarrior and the amplitude level of the music played is displayed on the 8 LEDs of the kit. Selection of the filter using the CLI. The filters are generated using the MatLab.

The project consists of the following setup:
MySQL DB - For storing the WAV files of the ringtones, Filters that can be applied to the ringtone while playing or before starting the play. 
Eclipse IDE
IOWarrior API
PortAudio Stream API
Separate DB classes for the filter database and the Sound Database
A separate UserInterface Class which handles all the requests from the user
MVC model-based application
Separate Exception Handlings class and also the file handling classes are created
