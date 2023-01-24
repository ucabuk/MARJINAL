# **MARJINALv0.1**

### **REQUIRED DLL FILES**

Note that you have to download those dll files to run the executable file.

libatomic-1.dll, libgcc_s_seh-1.dll, libgfortran-4.dll, libgomp-1.dll, libopencv_calib3d330.dll, libopencv_core330.dll, libopencv_dnn330.dll, libopencv_features2d330.dll, libopencv_flann330.dll, libopencv_highgui330.dll, libopencv_imgcodecs330.dll, libopencv_imgproc330.dll, libopencv_ml330.dll, libopencv_objdetect330.dlllibopencv_objdetect330.dll, libopencv_photo330.dll, libopencv_shape330.dll, libopencv_stitching330.dll, libopencv_superres330.dll, libquadmath-0.dll, libssp-0.dll, libstdc++-6.dll, libwinpthread-1.dll


## **DOCUMENTATION**

Marjinal can analyze the objects one by one, not merged objects in a single frame. Therefore, there should be only one object in each frame. 

Secondly, if Marjinal suddenly closes while running, this may be due to a high dimensional image (very high size) between samples.

### **HOW TO PREPARE DATASET**

1. Samples are firstly scanned in black and white one by one. (Min. recommended resolution: 300 dpi)  
2. Make sure that the samples are not folded and incomplete any parts regarding their structure since every margin is important for the analysis. Moreover, since the folding causes shadow, this can affect the results. So It may be biased.

### **HOW TO RUN MARJINAL**

1. please note that Marjinal can be only executed on Windows 64-bit system. After downloading the zip file, please unzip it.  
2. In the folder where Marjinal is located, there are files; those are Marjinal.exe, .dll extensions, and javascript files. Please do not remove or move any files to any directory. Keep them into the directory where it is located.  
3. The scanned samples are located in a folder (For example "A" folder) which must be found into Marjinal directory. If you download our OAK Dataset, firstly unzip and move the folder into Marjinal directory.  
4. Sample folder name: Please, use only English letters and do not use space between the characters for the name of the sample folder. Instead of this, you can use underscore or hyphen. 
5. Run Marjinal.exe. Once It is run, you will see the black screen. Type the name of the folder where the leaf samples are located. Once it starts, you can follow from the bar how far the process are progressed. 
6. When all the processes are done, you will be directed to the web browser to visualize the result. To get the tree result, firstly click "Draw Tree" button. Then, you can download the result by clicking "Download Tree" button.  
7. There will be a new folder, which is called as "Results_of_[name of the folder]", that is created after the process is done. 
8. Press any key on the black screen to turn off MARJINAL.
