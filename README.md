# HTTP log monitoring console program

## Content

- [Introduction](#introduction)
- [Requirements](#requirements)
- [How to build](#how-to-build)
- [How to use](#how-to-use)
- [How to launch unit tests](#how-to-launch-unit-tests)
- [Limitations](#limitations)

## Introduction

This tool reads a CSV-encoded HTTP access log from a file or the standard input stream to generate metrics and traffic alerts.

Example log file (first line is the header):
```
"remotehost","rfc931","authuser","date","request","status","bytes"
"10.0.0.1","-","apache",1549574332,"GET /api/user HTTP/1.0",200,1234
"10.0.0.4","-","apache",1549574333,"GET /report HTTP/1.0",200,1136
"10.0.0.1","-","apache",1549574334,"GET /api/user HTTP/1.0",200,1194
```
Full example : [file](Log_File.txt)

Metrics like "the number of hits" or "most hit section" are outputted every 10sc using the last 10sc logs. Alerts are evaluated in a sliding window of 2 min. An alert is fired if the average traffic per second exceeds a certain threshold. Another message is displayed when the traffic goes back under the threshold. The dates used are the log dates and not the current computer dates.

Example output:
![img](/img/outputs.PNG)


## Requirements

- Conan >= 1.39 and < 2.0
- CMake 3.5.1 or later

## How to build

Create a subfolder in your project folder to generate build files:
```
cd <project_folder>
mkdir build
cd build
```

Retrieve depencencies with conan:
```
conan install .. --build=missing -s build_type=Release
```

Generate the project with cmake (Command for Visual Studio 2022):
```
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
```

Build:
```
cmake --build . --config Release
```

## How to use

log_monitoring takes two optional parameters without any specific order:
- a file name: if not set, it will read the standard input 
- alert_th=x: a threshold of x hits per second for the traffic alert. The default value is 10.

Example:
```
log_monitoring Log_File.txt --alert_th=5
```

## How to launch unit tests

Execute log_monitoring_test

## Limitations
- No check on input log lines except the date: a complete validity check should be implemented
- Invalid logs are silently ignored: Users should be warned when logs are ignored, and an alert should be thrown when too many logs are invalid.
- Stream is read line by line: it is an acceptable simple implementation. In the case of high volume, a read-by-chunk could be more efficient.
- It has been designed as a light solution without large external dependencies like boost

