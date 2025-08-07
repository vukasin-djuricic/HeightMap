# HeightMap
3D computer graphics project

Heightmaps generated with (black-white):
https://manticorp.github.io/unrealheightmap/

Textures used from:
https://3dtextures.me

CONTROLS:
W, A, S, D - Movement
Q - lower Y scale (only when splitting into chunks is off)
E - raise Y scale (only when splitting into chunks is off)
SPACE - go up
SHIFT - go down
LMB (HOLD) - Look around
I - Decrease terrain vertex density 
O - Original terrain vertex density
R - Toggle grid
P - Toggle splitting into chunks

# Idea:

# 🇬🇧 English:
The heightmap is loaded from an image.  
The height of each vertex is computed based on the grayscale intensity of pixels  
(ref: https://blogs.igalia.com/itoral/2016/10/13/opengl-terrain-renderer-rendering-the-terrain-mesh/).

The terrain is divided into **chunks** to make **Level of Detail (LOD)** implementation easier.  
Heightmaps of different vertex densities are generated at startup from these chunks,  
and appropriate versions are rendered depending on the camera distance.  
Chunks whose centers are within a certain `dxz` distance from the camera are rendered at maximum resolution,  
while others use lower-density meshes.

### Texturing

The terrain uses **multiple texture layers** based on elevation (ref: https://www.mbsoftworks.sk/tutorials/opengl4/018-heightmap-pt3-multiple-layers/#google_vignette):  
1. Lava (lowest elevation)  
2. Interpolated blend between lava and rock  
3. Rock (higher elevation)

Normal maps are blended in a similar way.  
They are used to increase photorealism without increasing polygon count.  
Normal mapping is calculated using tangent, bitangent, and normal vectors  
(ref: https://learnopengl.com/Advanced-Lighting/Normal-Mapping).

The lighting model used is basic **Lambertian diffuse** shading.


# 🇷🇸 Srpski:
Height mapa se ucitava sa slike.
Visina vertex-a se racuna u odnosu na intenzitet pixela sa slike ( ref: https://blogs.igalia.com/itoral/2016/10/13/opengl-terrain-renderer-rendering-the-terrain-mesh/ ).
Height mapa je organizovana u chunk-ove koji sluze da bi se lakse implementirao LOD (level of detail).
Height mape razlicitih gustina se generisu na pocetku (iz chunkova), i prikazuju se prilikom renderovanja potrebni chunkovi (low/high res u zavisnosti od udaljenosti).
Chunkovi ciji centri su udaljeni za manje od proizvoljnog dxz od polozaja kamere ce biti u maksimalnoj gustini vrhova (vertices), dok ce ostali chunkovi imati manju gustinu vrhova.
Teksture - Teren ima vise tekstura u zavisnosti od visine (ref: https://www.mbsoftworks.sk/tutorials/opengl4/018-heightmap-pt3-multiple-layers/#google_vignette ).
1. nivo lava
2. nivo interpolacija izmedju lave i kamena
3. nivo kamen
Na slican nacin se interpoliraju i normal mape.
Normal mape su koriscene sa ciljem da povecaju foto-realisticnost i da pritom ne usloznjavaju kolicinu poligona i racunaju se pomocu tangent, bitangent i normalnog vektora (ref: https://learnopengl.com/Advanced-Lighting/Normal-Mapping ). Koristi se najjednostavniji Lambertov model za difuzno osvetljenje.
