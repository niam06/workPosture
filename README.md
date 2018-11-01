# workPosture
OpenCV-based C++ app for maintaining proper work posture
Poor posture while sitting and sitting for long periods of times lead to health problems. This program provides the length of time
the user has been sitting continuously and also provides a quality measure of sitting posture. This is done by applying HaarCascade
on pictures taken by webcam to identify whether the user is sitting in front or not, and by using a simple 1 minute training phase 
that provides a discriminative model to identify improver posture. 
