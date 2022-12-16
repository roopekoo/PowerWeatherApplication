# Power/weather data application
This was a 3-person group project to design a weather application that visualizes fetched data from FINGRID and FMI.

Software Design COMP.SE.110

Pulled from GitLab

MainWindow:
![](https://i.imgur.com/R5cfp7z.png)
## Dependencies:
- Qt-Creator
  - Qt 5.14 or above (Qt 6 seems to not work properly)
  - Qt Charts
  - Qt Network*
- OpenSSL installed (preferred version 1.1.1)

**This add-on may not be required to be installed separately if the feature comes with the main installation.*

- Doxygen

## How to run:
- Open the project from PWAnalyzer/PWAnalyzer.pro in Qt-Creator
- Select a compiler for the project. Any compiler should work. At least the following have been used in development:
  - GCC 64bit
  - MinGW 64-bit
- Build and run the project

- In the project root directory where the Doxyfile is located,
  - run $ doxygen
  - Open ./html/index.html in a web browser.

## Known issues:
None

