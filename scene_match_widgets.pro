QT += core gui widgets concurrent

# Specify C++ standard
CONFIG += c++17

# Include paths for OpenCV
INCLUDEPATH += /usr/local/include/opencv4

# Link libraries for OpenCV
LIBS += -L/usr/local/lib \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_features2d \
        -lopencv_calib3d \
        -lopencv_xfeatures2d \
        -lopencv_imgcodecs

# Add other project-specific files
SOURCES += \
    logwindow.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    logwindow.h \
    mainwindow.h

FORMS += \
    mainwindow.ui


