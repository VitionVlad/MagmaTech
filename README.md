# MagmaTech
MagmaTech is a free c++ vulkan based engine for 3d applications.  
it features:  
texture mapping (mipmap included)  
model loader (obj files)  
texture loader (ppm files)  
shadowmap  
resolution scale  
full android support(at least support for android 14 and adreno gpus)  
cubemap  
![Screenshot 2023-10-18 193733](https://github.com/VitionVlad/MagmaTech/assets/48290199/7f332c68-cb84-44a0-bce3-2fe7303679dd)
shadow acne is caused by the model, you can ad bias in shader, or use correct models to avoid it  
![Screenshot 2023-10-18 193721](https://github.com/VitionVlad/MagmaTech/assets/48290199/b8f409f6-b6f3-42f3-8efa-0e4d23fdac89)
cubemaps  
![Screenshot_20231018-203847](https://github.com/VitionVlad/MagmaTech/assets/48290199/4db58c38-52a2-478a-ba80-139ee6edee60)
same scene running on android phone (google pixel 5, android 14)  
![image](https://github.com/VitionVlad/MagmaTech/assets/48290199/d3ba1ca2-5ded-46ea-9742-038d5e489653)  
working text!
![image](https://github.com/VitionVlad/MagmaTech/assets/48290199/ce8b9e1a-1952-4e1a-bfcd-b970d83695f5)  
Working on Linux!!!
# Audio
audio in engine is based on miniaudio library, so it can work on windows, linux, android, and a lot of other thing, i personally tested on android and windows, it works! simple spacial audio system is included, but you can turn it off  
# Physics
phyisic engine is completly my development, it cannot calculate hard physic operations, it is only for collision, wich is calculated per vertex, and works automatically with every mesh, indiferent of shape, only requierent that could be is or to make player borders bigger, or to place more vertices, and destructions, you can create bombs, that have a radius, all vertices in that radius will be destroyed (in fact teleported to null coordinates)
![image](https://github.com/VitionVlad/MagmaTech/assets/48290199/20fef140-3902-4aff-8f29-015422f005f0)  
