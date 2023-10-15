# MagmaTech
MagmaTech is a free c++ vulkan based engine for 3d applications.  
it features:  
texture mapping (mipmap included)  
model loader (obj files)  
texture loader (ppm files)  
shadowmap pass  
resolution scale  
full android support  
![image](https://github.com/VitionVlad/MagmaTech/assets/48290199/b7573969-a2b8-4968-9247-cf7192148da3)  
example of scene working on windows  
![Screenshot_20231015-112322](https://github.com/VitionVlad/MagmaTech/assets/48290199/c9d2caad-e6b5-462b-83c2-d80eaf726e04)  
same scene running on android phone (google pixel 5, android 14)  
# Important note for android  
GLM_FORCE_DEFAULT_ALIGNED_GENTYPES was not working on android, to fix this you can make a modification into glm:  
![image](https://github.com/VitionVlad/MagmaTech/assets/48290199/eacaa7c7-76c7-4cb5-918c-212333ee259e)  
instead of:  
![image](https://github.com/VitionVlad/MagmaTech/assets/48290199/c81b4813-e15e-4434-a237-d1ff80339ab6)  
it is working perfectly fine, as seen on screenshot upper.  
